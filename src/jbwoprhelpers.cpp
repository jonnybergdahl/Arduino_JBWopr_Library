/// @file jbwoprhelpers.cpp
/// @author Jonny Bergdahl
/// @brief Source file for the JBWopr library.
/// @details Contains declarations for the base JBWoprDevice class.
/// This code is distributed under the MIT License. See LICENSE for details.
/// @date Created: 2023-10-02
/// @copyright Copyright© 2023, Jonny Bergdahl
#include "jbwoprhelpers.h"
#include "tz_data.h"
#include <WiFi.h>
#include <HTTPClient.h>

bool JBTimeHelper::_isInitialized = false;
std::string JBTimeHelper::_ntpServer = "";
std::string JBTimeHelper::_tzName = "";
JBLogger* JBTimeHelper::_log = nullptr;

void JBTimeHelper::configure(JBLogger* log, std::string ntpServer, std::string tzName) {
	_log = log;
	_ntpServer = ntpServer;
	_tzName = tzName;
}

bool JBTimeHelper::getTime(tm* info) {

	bool hasWiFi = WiFi.status() == WL_CONNECTED;

	if (hasWiFi && !_isInitialized) {
		_log->trace("Obtain local time");

		if (!_tzName.empty()) {
			std::string tzString = "";
			for (int i = 0; i < TZ_DATA_COUNT; i++) {
				if (_tzName == TZ_DATA[i].n) {
					tzString = TZ_DATA[i].v;
					break;
				}
			}

			if (!tzString.empty()) {
				_log->trace("Using TZ string: %s (%s)", tzString.c_str(), _tzName.c_str());
				configTzTime(tzString.c_str(), _ntpServer.c_str());
			}
			else {
				_log->warning("Timezone name not found: %s. Using UTC.", _tzName.c_str());
				configTime(0, 0, _ntpServer.c_str());
			}
		}
		else {
			_log->trace("Using UTC");
			configTime(0, 0, _ntpServer.c_str());
		}
		_isInitialized = true;
	}

	// We use a shorter timeout for non WiFi, the timeout is just to wait for the
	// NTP server response initiated by the call to configTime above
	if (!getLocalTime(info, hasWiFi ? 10000 : 1000)) {
		_log->error("Failed to obtain time");
		return false;
	}
	return true;
}

bool JBTimeHelper::setTime(const std::string currentDateTime) {
	struct tm tm = {};
	_log->trace(currentDateTime);
	// Parse ISO 8601: "YYYY-MM-DDTHH:MM:SS"
	if (sscanf(currentDateTime.c_str(), "%4d-%2d-%2dT%2d:%2d:%2d",
			   &tm.tm_year, &tm.tm_mon, &tm.tm_mday,
			   &tm.tm_hour, &tm.tm_min, &tm.tm_sec) != 6) {
		_log->error("Failed to parse currentDateTime value, check format");
		return false; // Failed to parse
	}
	_log->trace("Parsed time: %04d-%02d-%02d %02d:%02d:%02d",
				tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
				tm.tm_hour, tm.tm_min, tm.tm_sec);

	tm.tm_year -= 1900; // tm_year is years since 1900
	tm.tm_mon  -= 1;    // tm_mon is 0-based (0 = Jan)

	time_t t = mktime(&tm);
	if (t == -1) {
		_log->error("Failed to set time, check value");
		return false; // mktime failed
	}

	struct timeval now = { .tv_sec = t };
	if (!settimeofday(&now, nullptr))
	{
		_log->error("settimeofday() failed");
		return false;
	}
	return true;
}

std::string JBStringHelper::getCenteredString(std::string str, uint32_t length, char padChar) {
	if (str.length() > length) {
		return str;
	}

	uint32_t padSize = (length - str.length()) / 2;
	uint32_t padSizeLeft = padSize;
	uint32_t padSizeRight = padSize;

	std::string result;
	result.append(padSizeLeft, padChar);
	result.append(str);
	result.append(padSizeRight, padChar);

	return result;
}

std::string JBStringHelper::getRightAlignedString(std::string str, uint32_t length, char padChar) {
	if (str.length() > length) {
		return str;
	}

	uint32_t padSize = length - str.length();
	std::string result;
	result.append(padSize, padChar);
	result.append(str);

	return result;
}

std::string JBStringHelper::rgbToString(uint32_t rgbValue) {
	uint32_t red = (rgbValue >> 16) & 0xFF;
	uint32_t green = (rgbValue >> 8) & 0xFF;
	uint32_t blue = rgbValue & 0xFF;

	std::stringstream ss;
	ss << red << "," << green << "," << blue;
	return ss.str();
}

uint32_t JBStringHelper::stringToRgb(std::string rgbString) {
	if (rgbString.find(",") != 0) {
		uint32_t firstIndex = rgbString.find(",");
		uint32_t lastIndex = rgbString.find_last_of(",");
		std::string red = rgbString.substr(0, firstIndex);
		std::string green = rgbString.substr(firstIndex + 1, lastIndex - firstIndex - 1);
		std::string blue = rgbString.substr(lastIndex + 1);
		return atoi(red.c_str()) << 16 | atoi(green.c_str()) << 8 | atoi(blue.c_str());
	}
	return 0;
}

