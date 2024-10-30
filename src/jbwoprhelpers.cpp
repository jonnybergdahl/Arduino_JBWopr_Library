/// @file jbwoprhelpers.cpp
/// @author Jonny Bergdahl
/// @brief Source file for the JBWopr library.
/// @details Contains declarations for the base JBWoprDevice class.
/// This code is distributed under the MIT License. See LICENSE for details.
/// @date Created: 2023-10-02
/// @copyright Copyright© 2023, Jonny Bergdahl
#include "jbwoprhelpers.h"
#include <WiFi.h>
#include <HTTPClient.h>

bool JBTimeHelper::_isInitialized = false;
std::string JBTimeHelper::_ntpServer = "";
std::string JBTimeHelper::_timeOffsetString = "";
JBLogger* JBTimeHelper::_log = nullptr;

void JBTimeHelper::configure(JBLogger* log, std::string ntpServer, std::string timeOffsetString) {
	_log = log;
	_ntpServer = ntpServer;
	_timeOffsetString = timeOffsetString;
}

int64_t JBTimeHelper::getUtcOffsetInSeconds() {
	HTTPClient client;

	_log->trace("Getting local time zone");
	std::string url = "https://ipapi.co/utc_offset/";
	client.begin(url.c_str());
	int statusCode = client.GET();
	_log->trace("Status code: %i", statusCode);
	if (statusCode != 200) {
		_log->error("Error getting local time zone, status code: %i\n", statusCode);
		return 0;
	}

	auto offset = std::string(client.getString().c_str());
	_log->trace("Offset: %s", offset.c_str());
	char sign = offset[0];
	uint32_t hours = std::stoi(offset.substr(1, 2));
	uint32_t minutes = std::stoi(offset.substr(4, 2));
	int64_t result = (hours * 3600) + minutes * 60.0;
	if (sign == '-') {
		result = -result;
	}
	_log->trace("Offset in seconds: %f", result);
	return result;
}

bool JBTimeHelper::getTime(tm* info) {

	bool hasWiFi = WiFi.status() == WL_CONNECTED;

	if (hasWiFi && !_isInitialized) {
		_log->trace("Obtain time offset and local time");
		int64_t offset;
		if (_timeOffsetString.empty())
		{
			// Empty string means auto
			offset = getUtcOffsetInSeconds();
		}
		else {
			// Can't use stof() as it throws exceptions
			char * pEnd = nullptr;
			float offsetHours = strtof(_timeOffsetString.c_str(), &pEnd);
			_log->trace("Converting %s to time offset hours: %f", _timeOffsetString.c_str(), offsetHours);
			if (!*pEnd) {
				// Convert to seconds
				offset = int64_t(offsetHours * 60 * 60);
			}
			else { // error was detected
				offset = 0;
			}
		}
		configTime(offset, 0, _ntpServer.c_str());
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

