/// @file jbwoprhelpers.cpp
/// @author Jonny Bergdahl
/// @brief Source file for the JBWopr library.
/// @details Contains declarations for the base JBWoprDevice class.
/// This code is distributed under the MIT License. See LICENSE for details.
/// @date Created: 2023-10-02
/// @copyright Copyright© 2023, Jonny Bergdahl
///#include "jbwoprhelpers.h"
#include <JBLogger.h>
#include <WiFi.h>
#include <HTTPClient.h>

bool JBTimeHelper::_isInitialized = false;

uint32_t JBTimeHelper::getUtcOffsetInSeconds() {
	HTTPClient client;
	JBLogger log("timehelper");

	log.trace("Getting local time zone");
	std::string url = "https://ipapi.co/utc_offset/";
	client.begin(url.c_str());
	int statusCode = client.GET();
	log.trace("Status code: %i", statusCode);
	if (statusCode != 200) {
		log.error("Error getting local time zone, status code: %i\n", statusCode);
		return 0;
	}

	auto offset = std::string(client.getString().c_str());
	log.trace("Offset: %s", offset.c_str());
	char sign = offset[0];
	int hours = std::stoi(offset.substr(1, 2));
	int minutes = std::stoi(offset.substr(4, 2));
	float result = hours * 3600 +  + minutes * 60.0;
	if (sign == '-') {
		result = -result;
	}
	log.trace("Offset in seconds: %f", result);
	return result;
}

bool JBTimeHelper::getTime(tm* info)
{
	JBLogger log("timehelper");
	bool hasWiFi = WiFi.status() == WL_CONNECTED;
	if (hasWiFi && !_isInitialized) {
		log.trace("Obtain time offset and local time");
		float offset = getUtcOffsetInSeconds();
		configTime(offset, 0, "pool.ntp.org");
		_isInitialized = true;
	}
	// We use a shorter timeout for non WiFi, the timeout is just to wait for the
	// NTP server response initiated by the call to configTime above
	if (!getLocalTime(info, hasWiFi ? 10000 : 1000)) {
		log.error("Failed to obtain time");
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

