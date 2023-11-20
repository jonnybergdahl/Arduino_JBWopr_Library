/// @file jbwoprha.h
/// @author Jonny Bergdahl
/// @brief Main source file for the JBWopr library.
/// @details Contains declarations for the main JBWoprHaDevice class.
/// This code is distributed under the MIT License. See LICENSE for details.
/// @date Created: 2023-10-25
/// @copyright Copyright© 2023, Jonny Bergdahl
///
#include "jbwoprha.h"

// ====================================================================
// General
//
JBWoprHADevice::JBWoprHADevice() :
	JBWoprMqttDevice(),
		_haConfig {
		true,
		"homeassistant"
	},
	_log { new JBLogger("woprha", LogLevel::LOG_LEVEL_WARNING) }
{}

bool JBWoprHADevice::begin(JBWoprBoardVariant variant) {
	return JBWoprMqttDevice::begin(variant);
}

bool JBWoprHADevice::begin(JBWoprBoardVariant variant, JBWoprBoardPins pins) {
	JBWoprMqttDevice::begin(variant, pins);

	if (!_mqttConfig.useMqtt) {
		_log->warning("MQTT is not enabled");
		return false;
	}

	if (!_haConfig.useHomeAssistant) {
		_log->warning("Home Assistant is not enabled");
		return false;
	}

	_log->trace("Publish Home Assistant Discovery");
	if (!_homeAssistantSendDiscovery()) {
		_log->error("Failed to send Home Assistant discovery");
		return false;
	}

	return true;
}

void JBWoprHADevice::loop() {
	JBWoprMqttDevice::loop();

	if (_publishHomeAssistantDiscovery) {
		_homeAssistantSendDiscovery();
	}
}

// ====================================================================
// Logger
//
void JBWoprHADevice::setLogLevel(LogLevel level) {
	JBWoprMqttDevice::setLogLevel(level);
	_log->setLogLevel(level);
}

LogLevel JBWoprHADevice::getLogLevel() {
	return _log->getLogLevel();
}

// ====================================================================
// Configuration
//
void JBWoprHADevice::_setConfigFromJsonDocument(const DynamicJsonDocument& jsonDoc) {
	JBWoprMqttDevice::_setConfigFromJsonDocument(jsonDoc);

	if (!jsonDoc[CONF_HA_USE_HOME_ASSISTANT_KEY].isNull()) {
		_haConfig.useHomeAssistant = jsonDoc[CONF_HA_USE_HOME_ASSISTANT_KEY].as<bool>();
	}
	if (!jsonDoc[CONF_HA_DISCOVERY_PREFIX_KEY].isNull()) {
		_haConfig.homeAssistantDiscoveryPrefix = jsonDoc[CONF_HA_DISCOVERY_PREFIX_KEY].as<std::string>();
	}
}

void JBWoprHADevice::_setJsonDocumentFromConfig(DynamicJsonDocument& jsonDoc) {
	JBWoprMqttDevice::_setJsonDocumentFromConfig(jsonDoc);
	jsonDoc[CONF_HA_USE_HOME_ASSISTANT_KEY] = _haConfig.useHomeAssistant;
	jsonDoc[CONF_HA_DISCOVERY_PREFIX_KEY] = _haConfig.homeAssistantDiscoveryPrefix;
}

// ====================================================================
// WiFiManager
//
void JBWoprHADevice::_setupWiFiManager() {
	JBWoprMqttDevice::_setupWiFiManager();

	auto wifiManager = _getWiFiManager();

	_homeAssistantTitleParam = new WiFiManagerParameter(HTML_HOME_ASSISTANT_TITLE);
	_useHomeAssistantParam = new WiFiManagerParameter(CONF_HA_USE_HOME_ASSISTANT_KEY, "Use Home Assistant", "T", 2, _haConfig.useHomeAssistant ? HTML_CHECKBOX_TRUE : HTML_CHECKBOX_FALSE, WFM_LABEL_AFTER);
	_homeAssistantDiscoveryPrefixParam = new WiFiManagerParameter(CONF_HA_DISCOVERY_PREFIX_KEY, "Home Assistant Discovery prefix", _haConfig.homeAssistantDiscoveryPrefix.c_str(), 40);
	_break3Param = new WiFiManagerParameter("<br/>");

	wifiManager->addParameter(_homeAssistantTitleParam);
	wifiManager->addParameter(_useHomeAssistantParam);
	wifiManager->addParameter(_break3Param);
	wifiManager->addParameter(_homeAssistantDiscoveryPrefixParam);

	wifiManager->setSaveParamsCallback(std::bind(&JBWoprHADevice::_saveParamsCallback, this));
}
void JBWoprHADevice::_saveParamsCallback() {
	JBWoprMqttDevice::_saveParamsCallback();
	_haConfig.useHomeAssistant = strncmp(_useHomeAssistantParam->getValue(), "T", 1) == 0;
	_haConfig.homeAssistantDiscoveryPrefix, std::string(_homeAssistantDiscoveryPrefixParam->getValue(), sizeof(_mqttConfig.mqttServerName));

}

// ====================================================================
// MQTT
//
bool JBWoprHADevice::_onMqttConnect() {
	if (!JBWoprMqttDevice::_onMqttConnect()) {
		return false;
	}

	if (_haConfig.useHomeAssistant) {
		_log->trace("Publishing Home Assistant availability");
		mqttPublishMessage("wopr/WOPR-471da0d8/availability", "online");
		_homeAssistantSendDiscovery();
	}
	return true;
}

// ====================================================================
// Home Assistant
//
bool JBWoprHADevice::_homeAssistantSendDiscovery() {
	//_log->trace("Publishing Home Assistant discovery messages");
	_log->error("HA Discovery not implemented");
	return false;
}

void JBWoprHADevice::_homeAssistantHandleCommand(std::string entity, std::string subEntity, std::string command, std::string payload) {
	_log->error("HA Command not implemented");
	_log->trace("Home Assistant command: %s/%s/%s:", entity.c_str(), subEntity.c_str(), command.c_str());
	_log->traceDump(payload.c_str(), payload.length());

}

