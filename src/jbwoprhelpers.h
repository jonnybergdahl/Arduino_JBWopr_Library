/// @file timehelper.h
/// @author Jonny Bergdahl
/// @brief Header file for the JBWopr library.
/// @details Contains helper class declarations for the JBWopr library.
/// This code is distributed under the MIT License. See LICENSE for details.
/// @date Created: 2023-10-27
/// @copyright Copyright© 2023, Jonny Bergdahl

#ifndef ARDUINO_WOPR_JBWOPRHELPERS_H
#define ARDUINO_WOPR_JBWOPRHELPERS_H

#include <string>
#include <sstream>
#include <time.h>
#include <jblogger.h>

/// @brief This enum contains text alignment options
enum JBTextAlignment {
	LEFT = 0,								///< Left align text
	CENTER,									///< Center text
	RIGHT									///< Right align text
};


/// @brief This class contains helper functions for time related tasks
class JBTimeHelper {
public:
	/// @brief configure helper
	/// @param config WiFi configuration
	static void configure(JBLogger* log, std::string ntpServer, std::string timeOffsetString);

	/// @brief Get UTC offset in seconds
	/// @details Uses https://ipapi.co/timezone/ to get the local UTC offset
	/// @return UTC offset in seconds
	static int64_t getUtcOffsetInSeconds();

	/// @brief Get local time
	/// @param ntpServer NTP server address
	/// @param info Pointer to tm struct
	/// @details Uses NTP server pool.ntp.org to set local time
	/// @return True if successful
	static bool getTime(tm* info);

private:
	static bool _isInitialized;						///< True if done initializing
	static std::string _ntpServer;					///< NTP server
	static std::string _timeOffsetString;			///< Time offset string
	static JBLogger* _log;							///< Local logger
};

/// @brief This class contains helper functions for string related tasks
class JBStringHelper {
public:
	/// @brief Get centered string
	/// @param str String to center
	/// @param length Length of centered string
	/// @param padChar Character to pad with, default is space
	static std::string getCenteredString(const std::string str, const uint32_t length, char padChar = ' ');

	/// @brief Get right aligned string
	/// @param str String to right align
	/// @param length Length of right aligned string
	/// @param padChar Character to pad with, default is space
	static std::string getRightAlignedString(std::string str, uint32_t length, char padChar = ' ');

	/// @brief Get hex coded color string
	/// @param rgbValue RGB value
	/// @return Hex coded color string
	static std::string rgbToString(uint32_t rgbValue);

	/// @brief Get RGB value from hex coded color string
	/// @param rgbString Hex coded color string
	/// @return RGB value
	static uint32_t stringToRgb(std::string rgbString);

};

#endif //ARDUINO_WOPR_JBWOPRHELPERS_H
