/// @file jbwoprha.h
/// @author Jonny Bergdahl
/// @brief Main source file for the JBWopr library.
/// @details Contains declarations for the main JBWoprHaDevice class.
/// This code is distributed under the MIT License. See LICENSE for details.
/// @date Created: 2023-10-25
/// @copyright Copyright© 2023, Jonny Bergdahl
///
#include "jbwoprha.h"
#include "ha/ha_abbr.h"
#include "ha/mdi_consts.h"

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
	if (!JBWoprMqttDevice::begin(variant, pins)) {
		return false;
	}

	if (!_haConfig.useHomeAssistant) {
		_log->warning("Home Assistant is not enabled");
		return false;
	}

	return true;
}

void JBWoprHADevice::loop() {
	JBWoprMqttDevice::loop();

	// Nothing to do here
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
void JBWoprHADevice::_setConfigFromJsonDocument(const JsonDocument &jsonDoc) {
	JBWoprMqttDevice::_setConfigFromJsonDocument(jsonDoc);

	if (!jsonDoc[JSON_KEY_HA_USE_HOME_ASSISTANT].isNull()) {
		_haConfig.useHomeAssistant = jsonDoc[JSON_KEY_HA_USE_HOME_ASSISTANT].as<bool>();
	}
	if (!jsonDoc[JSON_KEY_HA_DISCOVERY_PREFIX].isNull()) {
		_haConfig.homeAssistantDiscoveryPrefix = jsonDoc[JSON_KEY_HA_DISCOVERY_PREFIX].as<std::string>();
	}
}

void JBWoprHADevice::_setJsonDocumentFromConfig(JsonDocument &jsonDoc) {
	JBWoprMqttDevice::_setJsonDocumentFromConfig(jsonDoc);
	jsonDoc[JSON_KEY_HA_USE_HOME_ASSISTANT] = _haConfig.useHomeAssistant;
	jsonDoc[JSON_KEY_HA_DISCOVERY_PREFIX] = _haConfig.homeAssistantDiscoveryPrefix;
}

void JBWoprHADevice::_dumpConfig() {
	JBWoprMqttDevice::_dumpConfig();
	_log->trace("Home Assistant configuration:");
	_log->trace("  Use Home Assistant: %s", _haConfig.useHomeAssistant ? "true" : "false");
	_log->trace("  Discovery prefix: %s", _haConfig.homeAssistantDiscoveryPrefix.c_str());
}
// ====================================================================
// WiFiManager
//
void JBWoprHADevice::_setupWiFiManager() {
	JBWoprMqttDevice::_setupWiFiManager();

	auto wifiManager = _getWiFiManager();

	_homeAssistantTitleParam = new WiFiManagerParameter(HTML_HOME_ASSISTANT_TITLE);
	_useHomeAssistantParam = new WiFiManagerParameter(JSON_KEY_HA_USE_HOME_ASSISTANT, "Use Home Assistant", "T", 2, _haConfig.useHomeAssistant ? HTML_CHECKBOX_TRUE : HTML_CHECKBOX_FALSE, WFM_LABEL_AFTER);
	_homeAssistantDiscoveryPrefixParam = new WiFiManagerParameter(JSON_KEY_HA_DISCOVERY_PREFIX, "Home Assistant Discovery prefix", _haConfig.homeAssistantDiscoveryPrefix.c_str(), 40);
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
	_haConfig.homeAssistantDiscoveryPrefix = std::string(_homeAssistantDiscoveryPrefixParam->getValue());
}

// ====================================================================
// MQTT
//
bool JBWoprHADevice::_onMqttConnect() {
	if (!JBWoprMqttDevice::_onMqttConnect()) {
		return false;
	}

	if (_haConfig.useHomeAssistant) {
		if (!_homeAssistantSendDiscovery()) {
			_log->error("Failed to send Home Assistant discovery");
			return false;
		}
		_homeAssistantPublishDiagnostics();
		_homeAssistantPublishConfig();
		_homeAssistantPublishState();
		mqttPublishMessage(_getAvailabilityTopic().c_str(), "online");
	}

	return true;
}

// ====================================================================
// Home Assistant
//
bool JBWoprHADevice::_homeAssistantSendDiscovery() {
	std::string topic;

	_log->trace("Publishing Home Assistant discovery messages");

	// Diagnostics
	// (We use nested scopes here to solve the memory issue)
	{
		JsonDocument jsonDoc;
		topic = _getDiscoveryTopic(HA_COMPONENT_SENSOR, HA_DIAG_PREFIX, HA_DIAG_ENTITY_IP);
		_addDiscoveryPayload(jsonDoc,
							 "IP Address",
							 HA_DIAG_PREFIX,
							 HA_DIAG_ENTITY_IP,
							 JSON_KEY_HA_DIAG_ENTITY_IP,
							 MDI_ICON_IP_NETWORK);
		mqttPublishMessage(topic, jsonDoc, true);
	}

	{
		JsonDocument jsonDoc;
		topic = _getDiscoveryTopic(HA_COMPONENT_SENSOR, HA_DIAG_PREFIX, HA_DIAG_ENTITY_RSSI);
		_addDiscoveryPayload(jsonDoc,
							 "RSSI",
							 HA_DIAG_PREFIX,
							 HA_DIAG_ENTITY_RSSI,
							 JSON_KEY_HA_DIAG_ENTITY_RSSI,
							 MDI_ICON_WIFI,
							 "dB");
		mqttPublishMessage(topic, jsonDoc, true);
	}

	{
		JsonDocument jsonDoc;
		topic = _getDiscoveryTopic(HA_COMPONENT_SENSOR, HA_DIAG_PREFIX, HA_DIAG_ENTITY_RAM);
		_addDiscoveryPayload(jsonDoc,
							 "Free memory",
							 HA_DIAG_PREFIX,
							 HA_DIAG_ENTITY_RAM,
							 JSON_KEY_HA_DIAG_ENTITY_RAM,
							 MDI_ICON_MEMORY,
							 "B");
		mqttPublishMessage(topic, jsonDoc, true);
	}

	// Config
	{
		JsonDocument jsonDoc;
		topic = _getDiscoveryTopic(HA_COMPONENT_SELECT, HA_CONFIG_PREFIX, HA_CONF_ENTITY_TIME_FORMAT);
		_addDiscoveryPayload(jsonDoc,
							 "Time format",
							 ENTITY_NAME_CONFIG,
							 HA_CONF_ENTITY_TIME_FORMAT,
							 JSON_KEY_TIME_FORMAT,
							 MDI_ICON_CLOCK_DIGITAL);
		auto timeOptions = jsonDoc["options"].to<JsonArray>();
		timeOptions.add("%H %M %S");
		timeOptions.add("%H.%M.%S");
		timeOptions.add("%H-%M-%S");
		timeOptions.add("%H:%M:%S");
		timeOptions.add("%I %M %S %p");
		timeOptions.add("%I.%M.%S %p");
		timeOptions.add("%I:%M:%S %p");
		jsonDoc["command_topic"] = "wopr/" + _getDeviceName() + "/config/time_format/set";
		mqttPublishMessage(topic, jsonDoc, true);
	}

	{
		JsonDocument jsonDoc;
		topic = _getDiscoveryTopic(HA_COMPONENT_SELECT, HA_CONFIG_PREFIX, HA_CONF_ENTITY_DATE_FORMAT);
		_addDiscoveryPayload(jsonDoc,
							 "Date format",
							 HA_CONFIG_PREFIX,
							 HA_CONF_ENTITY_DATE_FORMAT,
							 JSON_KEY_DATE_FORMAT,
							 MDI_ICON_CLOCK_DIGITAL);
		auto dateOptions = jsonDoc["options"].to<JsonArray>();
		dateOptions.add("%Y-%m-%d");
		dateOptions.add("%m/%d/%Y");
		dateOptions.add("%d/%m/%Y");
		dateOptions.add("%d-%m-%Y");
		dateOptions.add("%d.%m.%Y");
		jsonDoc["command_topic"] = "wopr/" + _getDeviceName() + "/config/date_format/set";
		mqttPublishMessage(topic, jsonDoc, true);
	}

	{
		JsonDocument jsonDoc;
		topic = _getDiscoveryTopic(HA_COMPONENT_NUMBER, HA_CONFIG_PREFIX, HA_CONF_ENTITY_DISPLAY_BRIGHTNESS);
		_addDiscoveryPayload(jsonDoc,
							 "Display brightness",
							 HA_CONFIG_PREFIX,
							 HA_CONF_ENTITY_DISPLAY_BRIGHTNESS,
							 JSON_KEY_DISPLAY_BRIGHTNESS,
							 MDI_ICON_BRIGHTNESS_5,
							 "%");
		jsonDoc["min"] = 0;
		jsonDoc["max"] = 100;
		jsonDoc["mode"] = "box";
		jsonDoc["command_topic"] = "wopr/" + _getDeviceName() + "/config/display_brightness/set";
		mqttPublishMessage(topic, jsonDoc, true);
	}

	{
		JsonDocument jsonDoc;
		topic = _getDiscoveryTopic(HA_COMPONENT_NUMBER, HA_CONFIG_PREFIX, HA_CONF_ENTITY_DEFCON_BRIGHTNESS);
		_addDiscoveryPayload(jsonDoc,
							 "DEFCON brightness",
							 HA_CONFIG_PREFIX,
							 HA_CONF_ENTITY_DEFCON_BRIGHTNESS,
							 JSON_KEY_DEFCON_BRIGHTNESS,
							 MDI_ICON_BRIGHTNESS_5,
							 "%");
		jsonDoc["min"] = 0;
		jsonDoc["max"] = 100;
		jsonDoc["mode"] = "box";
		jsonDoc["command_topic"] = "wopr/" + _getDeviceName() + "/config/defcon_brightness/set";
		mqttPublishMessage(topic, jsonDoc, true);
	}

	{
		JsonDocument jsonDoc;
		topic = _getDiscoveryTopic(HA_COMPONENT_NUMBER, HA_CONFIG_PREFIX, HA_CONF_ENTITY_EFFECTS_TIMEOUT);
		_addDiscoveryPayload(jsonDoc,
							 "Effects timeout",
							 HA_CONFIG_PREFIX,
							 HA_CONF_ENTITY_EFFECTS_TIMEOUT,
							 JSON_KEY_EFFECTS_TIMEOUT,
							 MDI_ICON_TIMER_OUTLINE,
							 "s");
		jsonDoc["mode"] = "box";
		jsonDoc["command_topic"] = "wopr/" + _getDeviceName() + "/config/effects_timeout/set";
		mqttPublishMessage(topic, jsonDoc, true);
	}

	{
		JsonDocument jsonDoc;
		topic = _getDiscoveryTopic(HA_COMPONENT_SWITCH, HA_CONFIG_PREFIX, HA_CONF_ENTITY_WIFI_USE_WEB_PORTAL);
		_addDiscoveryPayload(jsonDoc,
							 "Use web portal",
							 HA_CONFIG_PREFIX,
							 HA_CONF_ENTITY_WIFI_USE_WEB_PORTAL,
							 JSON_KEY_WIFI_USE_WEB_PORTAL,
							 MDI_ICON_WEB);
		jsonDoc["payload_on"] = "True";
		jsonDoc["payload_off"] = "False";
		jsonDoc["command_topic"] = "wopr/" + _getDeviceName() + "/config/use_web_portal/set";
		mqttPublishMessage(topic, jsonDoc, true);
	}

	{
		JsonDocument jsonDoc;
		topic = "homeassistant/button/" + _getDeviceName() + "/device/config";
		jsonDoc["name"] = "Restart";
		_addDeviceData(jsonDoc);
		jsonDoc[HA_NAMES_ENTITY_CATEGORY] = HA_CONFIG_PREFIX;
		jsonDoc["unique_id"] = _getDeviceName() + "_" +"device";
		jsonDoc["object_id"] = _getDeviceName() + "_" +"device";
		_addAvailabilityData(jsonDoc);
		jsonDoc["command_topic"] = "wopr/" + _getDeviceName() + "/device/state/set";
		jsonDoc["payload_press"] = "restart";
		jsonDoc["icon"] = MDI_ICON_RESTART;
		mqttPublishMessage(topic, jsonDoc, true);
	}

	// Effects
	{
		JsonDocument jsonDoc;
		topic = "homeassistant/switch/" + _getDeviceName() + "/effect/config";
		jsonDoc["name"] = "Run effect";
		_addDeviceData(jsonDoc);
		jsonDoc["unique_id"] = _getDeviceName() + "_" + "effect";
		jsonDoc["object_id"] = _getDeviceName() + "_" + "effect";
		_addAvailabilityData(jsonDoc);
		jsonDoc["state_topic"] = "wopr/" + _getDeviceName() + "/effect/state";
		jsonDoc["command_topic"] = "wopr/" + _getDeviceName() + "/effect/state/set";
		auto effects = effectsGetRegisteredEffects();
		jsonDoc["icon"] = MDI_ICON_SCRIPT_OUTLINE;
		mqttPublishMessage(topic, jsonDoc, true);
	}

	{
		JsonDocument jsonDoc;
		topic = "homeassistant/select/" + _getDeviceName() + "/effect/config";
		jsonDoc["name"] = "Effect";
		_addDeviceData(jsonDoc);
		jsonDoc["unique_id"] = _getDeviceName() + "_effect_name";
		jsonDoc["object_id"] = _getDeviceName() + "_effect_name";
		_addAvailabilityData(jsonDoc);
		jsonDoc["state_topic"] = "wopr/" + _getDeviceName() + "/effect/name";
		jsonDoc["command_topic"] = "wopr/" + _getDeviceName() + "/effect/name/set";
		auto effects = effectsGetRegisteredEffects();
		auto options = jsonDoc["options"].to<JsonArray>();
		options.add("");
		for (auto& effect : effects) {
			options.add(effect->getName());
		}
		jsonDoc["icon"] = MDI_ICON_SCRIPT_OUTLINE;
		mqttPublishMessage(topic, jsonDoc, true);
	}

	// Display
	{
		JsonDocument jsonDoc;
		topic = "homeassistant/light/" + _getDeviceName() + "/display/config";
		jsonDoc["name"] = "Display";
		_addDeviceData(jsonDoc);
		jsonDoc["unique_id"] = _getDeviceName() + "_display";
		jsonDoc["object_id"] = _getDeviceName() + "_display";
		_addAvailabilityData(jsonDoc);
		jsonDoc["state_topic"] = "wopr/" + _getDeviceName() + "/display/state";
		jsonDoc["command_topic"] = "wopr/" + _getDeviceName() + "/display/state/set";
		jsonDoc["brightness_state_topic"] = "wopr/" + _getDeviceName() + "/display/brightness";
		jsonDoc["brightness_command_topic"] = "wopr/" + _getDeviceName() + "/display/brightness/set";
		jsonDoc["brightness_scale"] = 100;
		jsonDoc["icon"] = MDI_ICON_ALPHABETICAL_VARIANT;
		mqttPublishMessage(topic, jsonDoc, true);
	}
	// DEFCON
	{
		JsonDocument jsonDoc;
		topic = "homeassistant/light/" + _getDeviceName() + "/defcon/config";
		jsonDoc["name"] = "DEFCON";
		_addDeviceData(jsonDoc);
		jsonDoc["unique_id"] = _getDeviceName() + "_defcon";
		jsonDoc["object_id"] = _getDeviceName() + "_defcon";
		_addAvailabilityData(jsonDoc);
		jsonDoc["state_topic"] = "wopr/" + _getDeviceName() + "/defcon/state";
		jsonDoc["command_topic"] = "wopr/" + _getDeviceName() + "/defcon/state/set";
		jsonDoc["brightness_state_topic"] = "wopr/" + _getDeviceName() + "/defcon/brightness";
		jsonDoc["brightness_command_topic"] = "wopr/" + _getDeviceName() + "/defcon/brightness/set";
		jsonDoc["rgb_state_topic"] = "wopr/" + _getDeviceName() + "/defcon/color";
		jsonDoc["rgb_command_topic"] = "wopr/" + _getDeviceName() + "/defcon/color/set";
		jsonDoc["brightness_scale"] = 100;
		jsonDoc["icon"] = MDI_ICON_NUMERIC_5_BOX_OUTLINE;
		mqttPublishMessage(topic, jsonDoc, true);
	}

	{
		JsonDocument jsonDoc;
		topic = "homeassistant/select/" + _getDeviceName() + "/defcon_level/config";
		jsonDoc["name"] = "DEFCON Level";
		_addDeviceData(jsonDoc);
		jsonDoc["unique_id"] = _getDeviceName() + "_defcon_level";
		jsonDoc["object_id"] = _getDeviceName() + "_defcon_level";
		_addAvailabilityData(jsonDoc);
		jsonDoc["state_topic"] = "wopr/" + _getDeviceName() + "/defcon/level";
		jsonDoc["command_topic"] = "wopr/" + _getDeviceName() + "/defcon/level/set";
		auto options = jsonDoc["options"].to<JsonArray>();
		for (auto& level : _defconNames) {
			options.add(level);
		}
		jsonDoc["icon"] = MDI_ICON_NUMERIC_5_BOX_OUTLINE;
		mqttPublishMessage(topic, jsonDoc, true);
	}

	{
		JsonDocument jsonDoc;
		topic = "homeassistant/text/" + _getDeviceName() + "/display_text/config";
		jsonDoc["name"] = "Display text";
		_addDeviceData(jsonDoc);
		jsonDoc["unique_id"] = _getDeviceName() + "_display_text";
		jsonDoc["object_id"] = _getDeviceName() + "_display_text";
		_addAvailabilityData(jsonDoc);
		jsonDoc["state_topic"] = "wopr/" + _getDeviceName() + "/display/text";
		jsonDoc["command_topic"] = "wopr/" + _getDeviceName() + "/display/text/set";
		jsonDoc["icon"] = MID_ICON_FORMAT_TEXT;
		mqttPublishMessage(topic, jsonDoc, true);
	}

	{
		JsonDocument jsonDoc;
		topic = "homeassistant/text/" + _getDeviceName() + "/display_scrolltext/config";
		jsonDoc["name"] = "Display scroll text";
		_addDeviceData(jsonDoc);
		jsonDoc["unique_id"] = _getDeviceName() + "_display_scrolltext";
		jsonDoc["object_id"] = _getDeviceName() + "_display_scrolltext";
		_addAvailabilityData(jsonDoc);
		jsonDoc["state_topic"] = "wopr/" + _getDeviceName() + "/display/scrolltext";
		jsonDoc["command_topic"] = "wopr/" + _getDeviceName() + "/display/scrolltext/set";
		jsonDoc["icon"] = MDI_ICON_TEXT_SHADOW;
		mqttPublishMessage(topic, jsonDoc, true);
	}

	return true;
}

bool JBWoprHADevice::_homeAssistantPublishDiagnostics() {
	_log->trace("Publishing Home Assistant diagnostics message");

	std::string diagnosticsTopic = _getTopic(ENTITY_NAME_DIAGNOSTIC, SUBENTITY_NAME_STATE);

	JsonDocument jsonDoc;
	jsonDoc[JSON_KEY_HA_DIAG_ENTITY_IP] = WiFi.localIP().toString();
	jsonDoc[JSON_KEY_HA_DIAG_ENTITY_RSSI] = WiFi.RSSI();
	jsonDoc[JSON_KEY_HA_DIAG_ENTITY_RAM] = ESP.getFreeHeap();
	jsonDoc[JSON_KEY_HA_DIAG_ENTITY_VERSION] = LIBRARY_VERSION;

	std::ostringstream oss;
	serializeJson(jsonDoc, oss);
	std::string payload = oss.str();

	mqttPublishMessage(diagnosticsTopic, payload);

	return true;
}

bool JBWoprHADevice::_homeAssistantPublishConfig() {
	JsonDocument jsonDoc;

	_log->trace("Publishing Home Assistant configuration");

	auto topic = _getTopic(HA_CONFIG_PREFIX, SUBENTITY_NAME_STATE);
	_setJsonDocumentFromConfig(jsonDoc);
	return mqttPublishMessage(topic, jsonDoc);
}

bool JBWoprHADevice::_homeAssistantPublishState() {
	_log->trace("Publishing Home Assistant state messages");

	// Effect
	mqttPublishMessage(_getTopic(ENTITY_NAME_EFFECT, SUBENTITY_NAME_STATE), effectsCurrentEffectIsRunning() ? "ON" : "OFF");
	mqttPublishMessage(_getTopic(ENTITY_NAME_EFFECT, SUBENTITY_NAME_NAME), "");
	// Display
	mqttPublishMessage(_getTopic(ENTITY_NAME_DISPLAY, SUBENTITY_NAME_STATE), _displayState ? "ON" : "OFF" );
	mqttPublishMessage(_getTopic(ENTITY_NAME_DISPLAY, SUBENTITY_NAME_TEXT), "");
	mqttPublishMessage(_getTopic(ENTITY_NAME_DISPLAY, SUBENTITY_NAME_BRIGHTNESS), std::to_string(_displayBrightness));
	mqttPublishMessage(_getTopic(ENTITY_NAME_DISPLAY, SUBENTITY_NAME_SCROLLTEXT), "");
	// DEFCON
	mqttPublishMessage(_getTopic(ENTITY_NAME_DEFCON, SUBENTITY_NAME_STATE), _defconState ? "ON" : "OFF");
	mqttPublishMessage(_getTopic(ENTITY_NAME_DEFCON, SUBENTITY_NAME_LEVEL), _defconNames.at((uint32_t)_defconLevel));
	mqttPublishMessage(_getTopic(ENTITY_NAME_DEFCON, SUBENTITY_NAME_BRIGHTNESS), std::to_string(_defconBrightness));
	mqttPublishMessage(_getTopic(ENTITY_NAME_DEFCON, SUBENTITY_NAME_COLOR), JBStringHelper::rgbToString(_defconLedsColor));

	return true;
}

std::string JBWoprHADevice::_getDiscoveryTopic(const std::string& component, const std::string& prefix,  const std::string& entity) {
	return _haConfig.homeAssistantDiscoveryPrefix + "/" + component + "/" + _getDeviceName() + "/" + prefix + "_" + entity + "/config";
}

void JBWoprHADevice::_addDiscoveryPayload(JsonDocument &jsonDoc,
										  const std::string& name,
										  const std::string& prefix,
										  const std::string& entity,
										  const std::string& templateValue,
										  const std::string& icon,
										  const std::string& unitOfMeasurement) {
	_addDeviceData(jsonDoc);
	_addAvailabilityData(jsonDoc);
	jsonDoc[HA_NAMES_NAME] = name;
	if (!prefix.empty()) {
		jsonDoc[HA_NAMES_ENTITY_CATEGORY] = prefix;
	}
	jsonDoc[HA_NAMES_UNIQUE_ID] = _getDeviceName() + "_" + prefix + "_" + entity;
	jsonDoc[HA_NAMES_OBJECT_ID] = _getDeviceName() + "_" +prefix + "_" + entity;
	jsonDoc[HA_NAMES_STATE_TOPIC] = _getTopic(prefix, SUBENTITY_NAME_STATE);
	jsonDoc[HA_NAMES_VALUE_TEMPLATE] = "{{ value_json." + templateValue + "}}";
	if (!icon.empty()) {
		jsonDoc[HA_NAMES_ICON] = icon;
	}
	if (!unitOfMeasurement.empty()) {
		jsonDoc[HA_NAMES_UNIT_OF_MEASUREMENT] = unitOfMeasurement;
	}
}

void JBWoprHADevice::_addDeviceData(JsonDocument &jsonDoc) {
	JsonObject device = jsonDoc["device"].to<JsonObject>();
	device["name"] = _getDeviceName();
	JsonArray identifiers = device["identifiers"].to<JsonArray>();
	identifiers.add(_getDeviceName());
	device["manufacturer"] = "Unexpected Maker";
	device["model"] = _woprVariant == JBWoprBoardVariant::ORIGINAL ? "W.O.P.R" : "W.O.P.R. Haxorz";
	device["sw_version"] = LIBRARY_VERSION;
	if (_wifiConfig.useWebPortal) {
		device["configuration_url"] = "http://" + WiFi.localIP().toString();
	}
}

void JBWoprHADevice::_addAvailabilityData(JsonDocument &jsonDoc) {
	JsonArray availability = jsonDoc["availability"].to<JsonArray>();
	auto data = availability.add<JsonObject>();
	data["topic"] = _getAvailabilityTopic();
}




