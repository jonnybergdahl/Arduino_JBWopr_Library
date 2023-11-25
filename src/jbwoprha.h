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
/// JBWoprHADevice configuration
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

	const char* JSON_KEY_HA_USE_HOME_ASSISTANT = "useHomeAssistant";	///< Use Home Assistant key name
	const char* JSON_KEY_HA_DISCOVERY_PREFIX = "discoveryPrefix";		///< Home Assistant discovery prefix key name

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
	const char* ENTITY_NAME_DIAGNOSTIC = "diagnostic";		///< Diagnostics entity name
	const char* ENTITY_NAME_CONFIG = "config";					///< Config entity name

	/// @brief Called when MQTT client get connected
	/// @ingroup MQTTGroup
	/// @details This method will be called when the MQTT client is connected.
	bool _onMqttConnect() override;

	// ====================================================================
	// Home Assistant
	//
	bool _publishHomeAssistantDiscovery;				///< True if Home Assistant discovery should be published

	const char* HA_DIAG_PREFIX = "diagnostic";							///< Diagnostic discovery prefix
	const char* HA_CONFIG_PREFIX = "config";							///< Config discovery prefix

	const char* HA_COMPONENT_LIGHT = "light";						///< Light component
	const char* HA_COMPONENT_NUMBER = "number";						///< Number component
	const char* HA_COMPONENT_SELECT = "select";						///< Select component
	const char* HA_COMPONENT_SENSOR = "sensor";						///< Sensor component
	const char* HA_COMPONENT_SWITCH = "switch";						///< Switch component

	const char* HA_DIAG_ENTITY_IP = "ip";								///< IP entity name
	const char* HA_DIAG_ENTITY_RSSI = "rssi";							///< RSSI entity name
	const char* HA_DIAG_ENTITY_RAM = "ram";								///< RAM entity name
	const char* HA_CONF_ENTITY_DATE_FORMAT = "date_format";				///< Date format entity name
	const char* HA_CONF_ENTITY_TIME_FORMAT = "time_format";				///< Time format entity name
	const char* HA_CONF_ENTITY_DISPLAY_BRIGHTNESS = "display_brightness";	///< Display brightness entity name
	const char* HA_CONF_ENTITY_DEFCON_BRIGHTNESS = "defcon_brightness";	///< DEFCON brightness entity name
	const char* HA_CONF_ENTITY_EFFECTS_TIMEOUT = "effects_timeout";		///< Effects timeout entity name
	const char* HA_CONF_ENTITY_WIFI_USE_WEB_PORTAL = "use_web_portal";	///< Use web portal entity name

	const char* JSON_KEY_HA_DIAG_ENTITY_IP = "ipAddress";						///< IP entity key name
	const char* JSON_KEY_HA_DIAG_ENTITY_RSSI = "rssi";							///< RSSI entity key name
	const char* JSON_KEY_HA_DIAG_ENTITY_RAM = "ram";							///< RAM entity key name
	const char* JSON_KEY_HA_DIAG_ENTITY_VERSION = "version";					///< Version entity key name

	/// @brief Send Home Assistant discovery
	bool _homeAssistantSendDiscovery();

	/// @brief Publish Home Assistant diagnostics
	bool _homeAssistantPublishDiagnostics();

	/// @brief Publish Home Assistant configuration
	bool _homeAssistantPublishConfig();

	/// @brief Publish Home Assistant state
	bool _homeAssistantPublishState();


	/// @brief Handle Home Assistant command
	void _homeAssistantHandleCommand(std::string entity, std::string subEntity, std::string command, std::string payload);

	/// @brief Get Home Assistant discovery topic
	/// @param entity Entity
	/// @param subEntity Sub entity
	/// @return Discovery topic
	std::string _getDiscoveryTopic(const std::string& component,
								   const std::string& prefix,
								   const std::string& entity);

	/// @brief Generate basic discovery message
	/// @param name Name
	/// @param prefix Prefix
	/// @param entity Entity
	/// @param templateValue Template
	/// @param icon Icon
	/// @param unitOfMeasurement Unit of measurement
	/// @return JSON document
	void _addDiscoveryPayload(DynamicJsonDocument& jsonDoc,
							  const std::string& name,
							  const std::string& prefix,
							  const std::string& entity,
							  const std::string& templateValue,
							  const std::string& icon,
							  const std::string& unitOfMeasurement = "");

	/// @brief Add device data to Home Assistant discovery message
	/// @param jsonDoc JSON document
	void _addDeviceData(DynamicJsonDocument& jsonDoc);

	/// @brief Add availability data to Home Assistant discovery message
	/// @param jsonDoc JSON document
	void _addAvailabilityData(DynamicJsonDocument& jsonDoc);

private:
	// ====================================================================
	// Logger
	//
	JBLogger* _log;									///< Logger
};


#endif //ARDUINO_WOPR_JBWOPRHA_H
