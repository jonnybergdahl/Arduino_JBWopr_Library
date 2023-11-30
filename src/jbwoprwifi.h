/// @file jbwoprwifi.h
/// @author Jonny Bergdahl
/// @brief Header file for the JBWOPR library.
/// @details Contains declarations for the JBWoprWiFiDevice class.
/// This code is distributed under the MIT License. See LICENSE for details.
/// @date Created: 2023-10-02
/// @copyright Copyright© 2023, Jonny Bergdahl
///
#ifndef ARDUINO_WOPR_JBWOPRWIFI_H
#define ARDUINO_WOPR_JBWOPRWIFI_H

#include "jbwopr.h"
#include <WiFiManager.h>                   	// https://github.com/tzapu/WiFiManager
#include <JBLogger.h>

// ====================================================================
//
// W.O.P.R. with WiFiManager support
//
// ====================================================================
/// @brief JBWoprWiFiDevice WiFi configuration
struct JBWoprWiFiConfiguration {
	std::string hostName;					///< Host name
	bool useWebPortal;                      ///< Use web portal
};

/// @brief W.O.P.R. WiFi device class
/// @details This class extends the JBWoprDevice class with WiFi support and
/// uses WiFiManager for easy configuration. It also supports a web portal for
/// configuration after the initial setup.
///
/// The configuration is saved to a file on the device, and is loaded on startup.
///
class JBWoprWiFiDevice: public JBWoprDevice {
public:
	// ====================================================================
	// General
	//
	/// @brief JBWoprWiFiDevice constructor
	JBWoprWiFiDevice();

	// Overrides
	/// @brief Initialize JBWoprWiFiDevice
	/// @ingroup GeneralGroup
	/// @details Use this method if you are using a tinyXxx board, it will
	/// assign the ESP32 pins automatically.
	/// @param variant Board version
	/// @return True if successful
	bool begin(JBWoprBoardVariant variant) override;

	/// @brief Initialize JBWoprWiFiDevice
	/// @ingroup GeneralGroup
	/// @param variant Board version
	/// @param pins Board pins
	/// @return True if successful
	bool begin(JBWoprBoardVariant variant, JBWoprBoardPins pins) override;

	/// @brief JBWoprWiFiDevice loop
	/// @ingroup GeneralGroup
	/// @details This method should be called from the main loop() method.
	void loop() override;

	// ====================================================================
	// Logger
	//
	/// @brief Set log level
	/// @ingroup LoggerGroup
	/// @details This method will set the log level for the logger.
	/// @param level Log level
	void setLogLevel(LogLevel level) override;

	/// @brief Get log level
	/// @ingroup LoggerGroup
	/// @return Log level
	LogLevel getLogLevel() override;

	// ====================================================================
	// Wifi
	//
	/// @brief Start WiFi
	/// @ingroup WiFiGroup
	/// @details This method will try to start WiFi with any saved credentials.
	///
	/// If not successful it will use WiFiManager to start an access point and
	/// offer a web portal for configuration.
	///
	/// If successful, it will optionally start a web configuration portal.
	///
	/// @return True if successful
	bool startWiFi();

	// WiFiManager
	/// @brief Start web portal
	/// @ingroup WiFiGroup
	/// @details This method will start a web portal for configuration.
	/// This will be done automatically if the configured.
	void webPortalStart();

	/// @brief Stop web portal
	/// @ingroup WiFiGroup
	/// @details This method will stop the web portal.
	void webPortalStop();

protected:
	// ====================================================================
	// Configuration
	//
	JBWoprWiFiConfiguration _wifiConfig;				///< WiFi configuration
	bool _shouldSaveConfig = false;						///< Flag to save configuration

	const char* CONFIG_FILE_NAME = "/config.json";		///< Configuration file name

	/// @brief Load configuration from file
	/// @ingroup ConfigurationGroup
	/// @details Called on startup.
	void _loadConfiguration();

	/// @brief Save configuration to file
	/// @ingroup ConfigurationGroup
	/// @details Called when Save button is pressed in the configuration web page.
	/// @ingroup ConfigurationGroup
	void _saveConfiguration();

	/// @brief Get configuration
	/// @ingroup ConfigurationGroup
	/// @details Called when JSON document have been loaded from file.
	/// @param jsonDoc JSON document
	virtual void _setConfigFromJsonDocument(const DynamicJsonDocument& jsonDoc);

	/// @brief Set configuration
	/// @ingroup ConfigurationGroup
	/// @details Called when JSON document is about to be saved to file.
	/// @param jsonDoc JSON document
	virtual void _setJsonDocumentFromConfig(DynamicJsonDocument& jsonDoc);

	// ====================================================================
	// WiFi
	//
	WiFiManager* _wifiManager;										///< WiFi manager
	std::string _apName = "";										///< AP name

	const char* WEB_PORTAL_PASSWORD = "wopr1234";       			///< AP portal password
	const char* JSON_KEY_EFFECTS_TIMEOUT = "effectsTimeout";		///< Effects timeout key name
	const char* JSON_KEY_TIME_FORMAT = "timeFormat";				///< Time format key name
	const char* JSON_KEY_DATE_FORMAT = "dateFormat";				///< Date Format key name
	const char* JSON_KEY_DEFCON_BRIGHTNESS = "defconBrightness";	///< DEFCON LEDs brightness key name
	const char* JSON_KEY_DISPLAY_BRIGHTNESS = "displayBrightness";	///< Display brightness key name
	const char* JSON_KEY_WIFI_HOST_NAME = "hostName";               ///< Host name key name
	const char* JSON_KEY_WIFI_USE_WEB_PORTAL = "useWebPortal";		///< Use portal key name

	const char* HTML_WOPR_TITLE = "<h2>W.O.P.R. settings</h2>";		///< Settings title
	const char* HTML_NETWORK_TITLE = "<h2>Network settings</h2>";	///< Network title
	const char* HTML_CHECKBOX_TRUE = "type=\"checkbox\" checked";   ///< HTML checkbox true
	const char* HTML_CHECKBOX_FALSE = "type=\"checkbox\"";          ///< HTML checkbox false

	WiFiManagerParameter* _woprTitleParam;							///< W.O.P.R. title parameter
	WiFiManagerParameter* _networkTitleParam;						///< Network title parameter
	WiFiManagerParameter* _breakParam;								///< Break parameter
	WiFiManagerParameter* _effectsTimeoutParam;						///< Effects timeout parameter
	WiFiManagerParameter* _timeFormatParam;							///< Time format parameter
	WiFiManagerParameter* _dateFormatParam;							///< Date format parameter
	WiFiManagerParameter* _displayBrightnessParam;					///< Display brightness parameter
	WiFiManagerParameter* _defconLedsBrightnessParam;				///< DEFCON LEDs brightness parameter

	WiFiManagerParameter* _hostNameParam;							///< Host name parameter
	WiFiManagerParameter* _useWebPortalParam;						///< Use web portal parameter

	char _effectsTimeoutValue[3];									///< Effects timeout value, set in WiFiManager callback
	char _defconLedsBrightnessValue[3];								///< DEFCON LEDs brightness value, set in WiFiManager callback
	char _displayBrightnessValue[3];								///< Display brightness value, set in WiFiManager callback

	/// @brief Get device name
	/// @ingroup WiFiGroup
	/// @return Device name
	std::string _getDeviceName();

	/// @brief Get WiFi manager
	/// @ingroup WiFiGroup
	/// @return WiFi manager
	virtual WiFiManager* _getWiFiManager();

	/// @brief Setup WiFiManager.
	/// @ingroup WiFiGroup
	virtual void _setupWiFiManager();

	/// @brief Save parameters callback
	/// @details Calleb by WiFiManager when Save button is pressed.
	/// @ingroup WiFiGroup
	virtual void _saveParamsCallback();

	/// @brief AP callback
	/// @ingroup WiFiGroup
	/// @details Called by WiFiManager if it starts up as an access point.
	/// @param wifiManager WiFi manager
	void _apCallback(WiFiManager* wifiManager);

	/// @brief Web server callback
	/// @ingroup WiFiGroup
	/// @details Called by WiFiManager when it starts up as a web server.
	void _webServerCallback();

private:
	// ====================================================================
	// Logger
	//
	JBLogger* _log;									///< Logger
};

#endif //ARDUINO_WOPR_JBWOPRWIFI_H
