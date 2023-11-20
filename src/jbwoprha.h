/// @file jbwoprha.h
/// @author Jonny Bergdahl
/// @brief Header file for the JBWopr library.
/// @details Contains declarations for the main JBWoprHaDevice class.
/// This code is distributed under the MIT License. See LICENSE for details.
/// @date Created: 2023-10-25
/// @copyright Copyright© 2023, Jonny Bergdahl
///
#ifndef ARDUINO_WOPR_JBWOPRHA_H
#define ARDUINO_WOPR_JBWOPRHA_H

#include "jbwoprmqtt.h"
#include "WiFiManager.h"
#include <jblogger.h>

// ====================================================================
//
// W.O.P.R. with WiFiManager, MQTT and Home Assistant support
//
// ====================================================================
/// Home Assistant specific configuration
struct JBWoprHAConfig {
	bool useHomeAssistant;
	std::string homeAssistantDiscoveryPrefix;
};

/// @brief W.O.P.R. WiFi, MQTT and HomeAssistant device class
/// @details This class extends the JBWoprMqttDevice class with Home Assistant support.
class JBWoprHADevice: public JBWoprMqttDevice {
public:
	// ====================================================================
	// General
	//
	/// @brief JBWoprHADevice constructor
	JBWoprHADevice();

	/// @brief Initialize JBWoprHADevice
	/// @details Use this method if you are using a tinyXxx board, it will
	/// assign the ESP32 pins automatically.
	/// @param variant Board version
	/// @return True if successful
	bool begin(JBWoprBoardVariant variant) override;

	/// @brief Initialize JBWoprHADevice
	/// @details Use this method if you are using a custom ESP32 board
	/// @param variant Board version
	/// @param pins Board pins
	/// @return True if successful
	bool begin(JBWoprBoardVariant variant, JBWoprBoardPins pins) override;

	/// @brief JBWoprHADevice loop
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

protected:
	// ====================================================================
	// Configuration
	//
	JBWoprHAConfig _haConfig;					///< Home Assistant configuration

	const char* CONF_HA_USE_HOME_ASSISTANT_KEY = "useHomeAssistant";	///< Use Home Assistant key name
	const char* CONF_HA_DISCOVERY_PREFIX_KEY = "discoveryPrefix";		//< Home Assistant discovery prefix key name

	/// @brief Set configuration from JSON document
	/// @param jsonDoc JSON document
	void _setConfigFromJsonDocument(const DynamicJsonDocument& jsonDoc) override;

	/// @brief Set JSON document from configuration
	/// @param jsonDoc JSON document
	void _setJsonDocumentFromConfig(DynamicJsonDocument& jsonDoc) override;

	// ====================================================================
	// Wifi
	//
	const char* HTML_HOME_ASSISTANT_TITLE = "<h2>Home Assistant settings</h2>";			///< MQTT title
	const char* HTML_CHECKBOX_TRUE = "type=\"checkbox\" checked";   ///< HTML checkbox true
	const char* HTML_CHECKBOX_FALSE = "type=\"checkbox\"";          ///< HTML checkbox false

	WiFiManagerParameter* _homeAssistantTitleParam;				///< Home Assistant title
	WiFiManagerParameter* _useHomeAssistantParam;				///< Use Home Assistant
	WiFiManagerParameter* _homeAssistantDiscoveryPrefixParam;	///< Home Assistant discovery prefix
	WiFiManagerParameter* _break3Param;							///< Break

	void _setupWiFiManager() override;
	void _saveParamsCallback() override;

	// ====================================================================
	// MQTT
	//
	/// @brief Called when MQTT client get connected
	/// @ingroup MQTTGroup
	/// @details This method will be called when the MQTT client is connected.
	bool _onMqttConnect() override;

	// ====================================================================
	// Home Assistant
	//
	bool _publishHomeAssistantDiscovery;				///< True if Home Assistant discovery should be published

	/// @brief Send Home Assistant discovery
	bool _homeAssistantSendDiscovery();

	/// @brief Handle Home Assistant command
	void _homeAssistantHandleCommand(std::string entity, std::string subEntity, std::string command, std::string payload);

private:
	// ====================================================================
	// Logger
	//
	JBLogger* _log;									///< Logger
};


#endif //ARDUINO_WOPR_JBWOPRHA_H
