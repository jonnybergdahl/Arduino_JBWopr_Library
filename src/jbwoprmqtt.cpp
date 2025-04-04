/// @file jbwoprmqtt.h
/// @author Jonny Bergdahl
/// @brief Main source file for the JBWopr library.
/// @details Contains declarations for the main JBWoprMqttDevice class.
/// This code is distributed under the MIT License. See LICENSE for details.
/// @date Created: 2023-10-25
/// @copyright Copyright© 2023, Jonny Bergdahl
///
#include "jbwoprmqtt.h"

// ====================================================================
// General
//
JBWoprMqttDevice::JBWoprMqttDevice() :
		JBWoprWiFiDevice(),
		_mqttConfig {
			false,						// useMqtt
			"",					// mqttServerName
			DEFAULT_MQTT_PORT,	// mqttServerPort
			"",					// mqttUserName
			"",					// mqttPassword
			DEFAULT_MQTT_PREFIX		// mqttPrefix
	},
	_log {new JBLogger("woprmqtt", LogLevel::LOG_LEVEL_TRACE) }
{}

bool JBWoprMqttDevice::begin(JBWoprBoardVariant variant) {
	return JBWoprWiFiDevice::begin(variant);
}

bool JBWoprMqttDevice::begin(JBWoprBoardVariant variant, JBWoprBoardPins pins)
{
	if (!JBWoprWiFiDevice::begin(variant, pins)) {
		return false;
	}

	// MQTT
	if (!_mqttConfig.useMqtt) {
		_log->trace("MQTT is not enabled");
		return true;
	}

	_log->info("Starting MQTT");
	displayShowText("Start MQTT");
	defconLedsSetColor(0x0000FF);

	if (!_mqttStart()) {
		_log->error("Failed to start MQTT");
		return false;
	}

	return true;
}

void JBWoprMqttDevice::loop() {
	if (!effectsCurrentEffectIsRunning() && _effectsCounter == 0) {
		mqttPublishMessage(_getTopic(ENTITY_NAME_EFFECT, SUBENTITY_NAME_STATE), STATE_OFF);
	}

	JBWoprWiFiDevice::loop();

	if (_mqttActive) {
		_mqttReconnect();
		if (_mqttClient->connected()) {
			_mqttClient->loop();
		}
	}
}

// ====================================================================
// Logger
//
void JBWoprMqttDevice::setLogLevel(LogLevel level) {
	JBWoprWiFiDevice::setLogLevel(level);
	_log->setLogLevel(level);
}

LogLevel JBWoprMqttDevice::getLogLevel() {
	return _log->getLogLevel();
}

// ====================================================================
// MQTT
//
bool JBWoprMqttDevice::mqttPublishMessage(const std::string& topic, JsonDocument &jsonDoc, bool retain) {
	char payload[1024];
	serializeJson(jsonDoc, payload);

	return mqttPublishMessage(topic.c_str(), payload, retain);
}

bool JBWoprMqttDevice::mqttPublishMessage(const std::string& topic, const std::string& payload, bool retain) {

	return mqttPublishMessage(topic.c_str(), payload.c_str(), retain);
}

bool JBWoprMqttDevice::mqttPublishMessage(const char* configTopic, const char* value, bool retain) {
	if (!_mqttActive) {
		_log->trace("MQTT not active, skipping publish");
		return false;
	}

	if (!_mqttClient->connected()) {
		_log->trace("MQTT not connected, skipping publish");
		return false;
	}
	if (!_mqttClient->publish(configTopic, value, retain))
	{
		_log->error("Failed to publish to MQTT topic");
		return false;
	}

	_log->trace("MQTT > %s %s:", configTopic, retain ? "(retain)" : "");
	_log->traceAsciiDump(value, strlen(value));

	return true;
}

// ====================================================================
// Effects
//
void JBWoprMqttDevice::effectsStartCurrentEffect() {
	JBWoprWiFiDevice::effectsStartCurrentEffect();
	mqttPublishMessage(_getTopic(ENTITY_NAME_EFFECT, SUBENTITY_NAME_STATE), STATE_ON);
}

void JBWoprMqttDevice::effectsStopCurrentEffect() {
	JBWoprWiFiDevice::effectsStopCurrentEffect();
	mqttPublishMessage(_getTopic(ENTITY_NAME_EFFECT, SUBENTITY_NAME_STATE), STATE_OFF);
}

void JBWoprMqttDevice::effectsStartEffect(JBWoprEffectBase *effect) {
	JBWoprWiFiDevice::effectsStartEffect(effect);
	mqttPublishMessage(_getTopic(ENTITY_NAME_EFFECT, SUBENTITY_NAME_NAME), effect->getName());
	mqttPublishMessage(_getTopic(ENTITY_NAME_EFFECT, SUBENTITY_NAME_STATE), STATE_ON);
}

void JBWoprMqttDevice::effectsStartEffect(const std::string& effect) {
	JBWoprWiFiDevice::effectsStartEffect(effect);
	mqttPublishMessage(_getTopic(ENTITY_NAME_EFFECT, SUBENTITY_NAME_NAME), effect);
	mqttPublishMessage(_getTopic(ENTITY_NAME_EFFECT, SUBENTITY_NAME_STATE), STATE_ON);
}

void JBWoprMqttDevice::effectsStartEffect(const char* effect) {
	JBWoprWiFiDevice::effectsStartEffect(effect);
	mqttPublishMessage(_getTopic(ENTITY_NAME_EFFECT, SUBENTITY_NAME_NAME), effect);
	mqttPublishMessage(_getTopic(ENTITY_NAME_EFFECT, SUBENTITY_NAME_STATE), STATE_ON);
}

// ====================================================================
// Display
//
void JBWoprMqttDevice::displaySetState(bool state) {
	JBWoprWiFiDevice::displaySetState(state);
	mqttPublishMessage(_getTopic(ENTITY_NAME_DISPLAY, SUBENTITY_NAME_STATE), state ? STATE_ON : STATE_OFF);
}

void JBWoprMqttDevice::displayClear() {
	JBWoprWiFiDevice::displayClear();
	mqttPublishMessage(_getTopic(ENTITY_NAME_DISPLAY, SUBENTITY_NAME_TEXT), "");
}

void JBWoprMqttDevice::displaySetBrightness(uint8_t val) {
	JBWoprWiFiDevice::displaySetBrightness(val);
	mqttPublishMessage(_getTopic(ENTITY_NAME_DISPLAY, SUBENTITY_NAME_BRIGHTNESS), std::to_string(val));
}

void JBWoprMqttDevice::displayShowText(const char* text, JBTextAlignment alignment) {
	JBWoprDevice::displayShowText(text, alignment);
	mqttPublishMessage(_getTopic(ENTITY_NAME_DISPLAY, SUBENTITY_NAME_TEXT), text);
}

void JBWoprMqttDevice::displayShowText(const std::string& text, JBTextAlignment alignment) {
	displayShowText(text.c_str(), alignment);
}

void JBWoprMqttDevice::displayShowText(const String& text, JBTextAlignment alignment) {
	displayShowText(text.c_str(), alignment);
}

void JBWoprMqttDevice::displayShowText(const char *text) {
	displayShowText(text, JBTextAlignment::LEFT);
}

void JBWoprMqttDevice::displayShowText(const std::string& text) {
	displayShowText(text.c_str(), JBTextAlignment::LEFT);
}

void JBWoprMqttDevice::displayShowText(const String& text) {
	displayShowText(text.c_str(), JBTextAlignment::LEFT);
}

void JBWoprMqttDevice::displayScrollText(const char* text, uint16_t delay_ms) {
	JBWoprDevice::displayScrollText(text, delay_ms);
	mqttPublishMessage(_getTopic(ENTITY_NAME_DISPLAY, SUBENTITY_NAME_SCROLLTEXT), text);
}

void JBWoprMqttDevice::displayScrollText(const std::string& text, uint16_t delay_ms) {
	displayScrollText(text.c_str(), delay_ms);
}

void JBWoprMqttDevice::displayScrollText(const String& text, uint16_t delay_ms) {
	displayScrollText(text.c_str(), delay_ms);
}

void JBWoprMqttDevice::displayScrollText(const char* text) {
	displayScrollText(text, 100);
}

void JBWoprMqttDevice::displayScrollText(const std::string& text) {
	displayScrollText(text.c_str(), 100);
}

void JBWoprMqttDevice::displayScrollText(const String& text) {
	displayScrollText(text.c_str(), 100);
}

// ====================================================================
// Defcon
//
void JBWoprMqttDevice::defconLedsSetState(bool state) {
	JBWoprWiFiDevice::defconLedsSetState(state);
	mqttPublishMessage(_getTopic(ENTITY_NAME_DEFCON, SUBENTITY_NAME_STATE), state ? STATE_ON : STATE_OFF);
}

void JBWoprMqttDevice::defconLedsSetDefconLevel(JBDefconLevel level) {
	JBWoprWiFiDevice::defconLedsSetDefconLevel(level);
	mqttPublishMessage(_getTopic(ENTITY_NAME_DEFCON, SUBENTITY_NAME_LEVEL), DEFCON_STRINGS[level]);
}
void JBWoprMqttDevice::defconLedsSetColor(uint32_t color) {
	JBWoprWiFiDevice::defconLedsSetColor(color);
	mqttPublishMessage(_getTopic(ENTITY_NAME_DEFCON, SUBENTITY_NAME_COLOR), JBStringHelper::rgbToString(color));
}

void JBWoprMqttDevice::defconLedsSetBrightness(uint8_t brightness) {
	JBWoprWiFiDevice::defconLedsSetBrightness(brightness);
	mqttPublishMessage(_getTopic(ENTITY_NAME_DEFCON, SUBENTITY_NAME_BRIGHTNESS), std::to_string(brightness));
}

void JBWoprMqttDevice::defconLedSetColor(JBDefconLevel level, uint32_t color) {
	JBWoprWiFiDevice::defconLedSetColor(level, color);
}

// ====================================================================
// Configuration
//
void JBWoprMqttDevice::_setConfigFromJsonDocument(const JsonDocument &jsonDoc) {
	JBWoprWiFiDevice::_setConfigFromJsonDocument(jsonDoc);
	if (!jsonDoc[JSON_KEY_MQTT_USE_MQTT].isNull()) {
		_mqttConfig.useMqtt = jsonDoc[JSON_KEY_MQTT_USE_MQTT].as<bool>();
	}
	if (!jsonDoc[JSON_KEY_MQTT_SERVER_NAME].isNull()) {
		_mqttConfig.mqttServerName = jsonDoc[JSON_KEY_MQTT_SERVER_NAME].as<std::string>();
	}
	if (!jsonDoc[JSON_KEY_MQTT_SERVER_PORT].isNull()) {
		_mqttConfig.mqttServerPort = jsonDoc[JSON_KEY_MQTT_SERVER_PORT].as<uint16_t>();
	}
	if (!jsonDoc[JSON_KEY_MQTT_USER_NAME].isNull()) {
		_mqttConfig.mqttUserName = jsonDoc[JSON_KEY_MQTT_USER_NAME].as<std::string>();
	}
	if (!jsonDoc[JSON_KEY_MQTT_PASSWORD].isNull()) {
		_mqttConfig.mqttPassword = jsonDoc[JSON_KEY_MQTT_PASSWORD].as<std::string>();
	}
	if (!jsonDoc[JSON_KEY_CONF_MQTT_PREFIX].isNull()) {
		_mqttConfig.mqttPrefix = jsonDoc[JSON_KEY_CONF_MQTT_PREFIX].as<std::string>();
	}
}

void JBWoprMqttDevice::_setJsonDocumentFromConfig(JsonDocument &jsonDoc) {
	JBWoprWiFiDevice::_setJsonDocumentFromConfig(jsonDoc);
	jsonDoc[JSON_KEY_MQTT_USE_MQTT] = _mqttConfig.useMqtt;
	jsonDoc[JSON_KEY_MQTT_SERVER_NAME] = _mqttConfig.mqttServerName;
	jsonDoc[JSON_KEY_MQTT_SERVER_PORT] = _mqttConfig.mqttServerPort;
	jsonDoc[JSON_KEY_MQTT_USER_NAME] = _mqttConfig.mqttUserName;
	jsonDoc[JSON_KEY_MQTT_PASSWORD] = _mqttConfig.mqttPassword;
	jsonDoc[JSON_KEY_CONF_MQTT_PREFIX] = _mqttConfig.mqttPrefix;
}

void JBWoprMqttDevice::_dumpConfig() {
	JBWoprWiFiDevice::_dumpConfig();
	_log->trace("MQTT configuration:");
	_log->trace("  Use MQTT: %s", _mqttConfig.useMqtt ? "True" : "False");
	_log->trace("  MQTT server name: %s", _mqttConfig.mqttServerName.c_str());
	_log->trace("  MQTT server port: %i", _mqttConfig.mqttServerPort);
	_log->trace("  MQTT user name: %s", _mqttConfig.mqttUserName.c_str());
	_log->trace("  MQTT password: ----------");
	_log->trace("  MQTT prefix: %s", _mqttConfig.mqttPrefix.c_str());
}

// ====================================================================
// WiFi
//
void JBWoprMqttDevice::_setupWiFiManager(){
	JBWoprWiFiDevice::_setupWiFiManager();

	auto wifiManager = _getWiFiManager();
	snprintf(_mqttServerPortValue, sizeof(_mqttServerPortValue), "%u", _mqttConfig.mqttServerPort);

	_mqttTitleParam = new WiFiManagerParameter(HTML_MQTT_TITLE);
	_break2Param = new WiFiManagerParameter("<br/>");
	_useMqttParam = new WiFiManagerParameter(JSON_KEY_MQTT_USE_MQTT, "Use MQTT", "T", 2, _mqttConfig.useMqtt ? HTML_CHECKBOX_TRUE : HTML_CHECKBOX_FALSE, WFM_LABEL_AFTER);
	_mqttServerNameParam = new WiFiManagerParameter(JSON_KEY_MQTT_SERVER_NAME, "MQTT server", _mqttConfig.mqttServerName.c_str(), 40);
	_mqttServerPortParam = new WiFiManagerParameter(JSON_KEY_MQTT_SERVER_PORT, "MQTT port", _mqttServerPortValue, 5);
	_mqttUserNameParam = new WiFiManagerParameter(JSON_KEY_MQTT_USER_NAME, "MQTT user name", _mqttConfig.mqttUserName.c_str(), 40);
	_mqttPasswordParam = new WiFiManagerParameter(JSON_KEY_MQTT_PASSWORD, "MQTT password", _mqttConfig.mqttPassword.c_str(), 40);
	_mqttPrefixParam = new WiFiManagerParameter(JSON_KEY_CONF_MQTT_PREFIX, "MQTT prefix", _mqttConfig.mqttPrefix.c_str(), 40);

	wifiManager->addParameter(_mqttTitleParam);
	wifiManager->addParameter(_useMqttParam);
	wifiManager->addParameter(_break2Param);
	wifiManager->addParameter(_mqttServerNameParam);
	wifiManager->addParameter(_mqttServerPortParam);
	wifiManager->addParameter(_mqttUserNameParam);
	wifiManager->addParameter(_mqttPasswordParam);
	wifiManager->addParameter(_mqttPrefixParam);

	wifiManager->setSaveParamsCallback(std::bind(&JBWoprMqttDevice::_saveParamsCallback, this));
}

void JBWoprMqttDevice::_saveParamsCallback() {
	JBWoprWiFiDevice::_saveParamsCallback();

	_mqttConfig.useMqtt = strncmp(_useMqttParam->getValue(), "T", 1) == 0;
	_mqttConfig.mqttServerName = std::string(_mqttServerNameParam->getValue());
	if (_mqttConfig.mqttServerName.length() == 0) {
		_mqttConfig.useMqtt = false;
	}
	_mqttConfig.mqttServerPort = atoi(_mqttServerPortParam->getValue());
	_mqttConfig.mqttUserName = std::string(_mqttUserNameParam->getValue());
	_mqttConfig.mqttPassword = std::string(_mqttPasswordParam->getValue());
	_mqttConfig.mqttPrefix = std::string(_mqttPrefixParam->getValue());
}

// ====================================================================
// MQTT
//
bool JBWoprMqttDevice::_mqttStart() {
	// Sanity check
	if (!_mqttConfig.useMqtt) {
		_log->trace("MQTT is not enabled");
		return false;
	}
	if (_mqttConfig.mqttServerName.length() == 0) {
		_log->error("MQTT server name is not set");
		return false;
	}

	_mqttClient = new PubSubClient(_mqttConfig.mqttServerName.c_str(),
								   _mqttConfig.mqttServerPort,
								   _wifiClient);
	_mqttActive = true;

	_log->trace("Starting MQTT, %s:%i", _mqttConfig.mqttServerName.c_str(), _mqttConfig.mqttServerPort);
	if (!_mqttClient->setBufferSize(1024))
	{
		_log->error("Failed to set MQTT buffer size");
		return false;
	}
	_mqttClient->setSocketTimeout(30);
	_mqttClient->setCallback(std::bind(&JBWoprMqttDevice::_mqttCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

	if (!_mqttReconnect()) {
		return false;
	}

	return true;
}

void JBWoprMqttDevice::_mqttStop() {
	if (_mqttClient->connected()) {
		_mqttClient->disconnect();
	}
	_log->debug("Disconnected from MQTT server");
}

bool JBWoprMqttDevice::_mqttReconnect() {
	const int maxRetries = 3;

	if (_mqttClient->connected()) {
		return true;
	}

	_log->debug("Connecting to MQTT server: %s:%i", _mqttConfig.mqttServerName.c_str(), _mqttConfig.mqttServerPort);

	for (int retry = 0; retry < maxRetries; ++retry) {
		if (!_mqttClient->connect(_getDeviceName().c_str(),
								  _mqttConfig.mqttUserName.c_str(),
								  _mqttConfig.mqttPassword.c_str(),
								  _getAvailabilityTopic().c_str(),
								  1,
								  true,
								  "offline")) {
			_log->error("Failed to connect to MQTT server, error %i", _mqttClient->state());
			delay(500);
		} else {
			if (!_onMqttConnect()) {
				return false;
			}
			break;
		}
	}

	_mqttActive = true;
	_log->debug("Connected to MQTT server");
	return true;
}

bool JBWoprMqttDevice::_onMqttConnect() {
	std::string subscribeTopic = _getSubscriptionTopic();
	_log->debug("Subscribing to MQTT topic: %s", subscribeTopic.c_str());
	if (!_mqttClient->subscribe(subscribeTopic.c_str())) {
		_log->error("Failed to subscribe to MQTT topic, error: %i", _mqttClient->state());
		return false;
	}
	mqttPublishMessage(_getAvailabilityTopic().c_str(), "online");
	return true;
}

void JBWoprMqttDevice::_mqttCallback(const char* topic, const byte* payload, unsigned int length){
	_log->trace("MQTT < Topic: %s", topic);
	_log->traceAsciiDump(payload, length);

	std::string entity;
	std::string subEntity;
	std::string command;
	std::string payloadStr(reinterpret_cast<const char*>(payload), length);

	std::stringstream ss(topic);
	std::string part;

	int i = 0;
	while (std::getline(ss, part, '/'))
	{
		switch (i) {
			case 2:
				entity = part;
				break;
			case 3:
				subEntity = part;
				break;
			case 4:
				command = part;
				break;
		}
		i++;
	}
	_handleCommand(entity, subEntity, command, payloadStr);
}

void JBWoprMqttDevice::_handleCommand(const std::string& entity, const std::string& subEntity, const std::string& command, const std::string& payload)
{
	if (entity == ENTITY_NAME_DEVICE) {
		_handleDeviceCommand(subEntity, command, payload);
	} else if (entity == ENTITY_NAME_EFFECT) {
		_handleEffectCommand(subEntity, command, payload);
	} else if (entity == ENTITY_NAME_DISPLAY) {
		_handleDisplayCommand(subEntity, command, payload);
	} else if (entity == ENTITY_NAME_DEFCON) {
		_handleDefconCommand(subEntity, command, payload);
	} else if (entity == ENTITY_NAME_CONFIG) {
		_handleConfigCommand(subEntity, command, payload);
	} else {
		_log->error("Unsupported entity: %s, %s, %s", entity.c_str(), subEntity.c_str(), command.c_str());
	}
}

void JBWoprMqttDevice::_handleDeviceCommand(const std::string &subEntity,
											const std::string &command,
											const std::string &payload) {
	if (subEntity == SUBENTITY_NAME_STATE) {
		if (command == COMMAND_SET) {
			if (payload == "restart") {
				_log->info("Restarting device");
				displayShowText("Restarting");
				defconLedsSetColor(0x0000FF);
				delay(1000);
				ESP.restart();
			} else {
				_log->error("Unsupported payload: %s, %s: %s", subEntity.c_str(), command.c_str(), payload.c_str());
			}
		} else {
			_log->error("Unsupported command: %s %s", subEntity.c_str(), command.c_str());
		}
	} else {
		_log->error("Unsupported command: %s %s", subEntity.c_str(), command.c_str());
	}

}

void JBWoprMqttDevice::_handleConfigCommand(const std::string &subEntity,
											const std::string &command,
											const std::string &payload) {
	if (command == COMMAND_SET) {
		if (subEntity == SUBENTITY_NAME_TIME_FORMAT) {
			_config.timeFormat = payload;
			_saveConfiguration();
		} else if (subEntity == SUBENTITY_NAME_DATE_FORMAT) {
			_config.dateFormat = payload;
			_saveConfiguration();
		} else if (subEntity == SUBENTITY_NAME_DISPLAY_BRIGHTNESS) {
			_config.displayBrightness = atoi(payload.c_str());
			_saveConfiguration();
		} else if (subEntity == SUBENTITY_NAME_DEFCON_BRIGHTNESS) {
			_config.defconLedsBrightness = atoi(payload.c_str());
			_saveConfiguration();
		} else if (subEntity == SUBENTITY_NAME_EFFECTS_TIMEOUT) {
			_config.effectsTimeout = atoi(payload.c_str());
			_saveConfiguration();
		} else if (subEntity == SUBENTITY_NAME_WIFI_USE_WEB_PORTAL) {
			_wifiConfig.useWebPortal = payload == "True";
			_saveConfiguration();
			if (_wifiConfig.useWebPortal) {
				webPortalStart();
			} else {
				webPortalStop();
			}
		} else {
			_log->error("Unsupported sub entity: %s", subEntity.c_str());
		}
	} else {
		_log->error("Unsupported command: %s %s", subEntity.c_str(), command.c_str());
	}
}

void JBWoprMqttDevice::_handleEffectCommand(const std::string& subEntity,
											const std::string& command,
											const std::string& payload) {
	if (subEntity == SUBENTITY_NAME_STATE) {
		if (command == COMMAND_SET) {
			if (!payload.empty()) {
				effectsStartEffect(payload);
			} else {
				effectsStopCurrentEffect();
			}
		} else {
			_log->error("Unsupported command: %s %s", subEntity.c_str(), command.c_str());
		}
	} else if (subEntity == SUBENTITY_NAME_NAME) {
		if (command == COMMAND_SET) {
			effectsStartEffect(payload.c_str());
		} else {
			_log->error("Unsupported command: %s %s", subEntity.c_str(), command.c_str());
		}
	} else {
		_log->error("Unsupported sub entity: %s", subEntity.c_str());
	}
}

void JBWoprMqttDevice::_handleDisplayCommand(const std::string& subEntity,
											 const std::string& command,
											 const std::string& payload) {
	if (subEntity == SUBENTITY_NAME_STATE) {
		if (command == COMMAND_SET) {
			if (payload == STATE_ON) {
				displaySetState(true);
			} else if (payload == STATE_OFF) {
				displaySetState(false);
			} else {
				_log->error("Unsupported payload: %s, %s: %s", subEntity.c_str(), command.c_str(), payload.c_str());
			}
		} else {
			_log->error("Unsupported command: %s %s", subEntity.c_str(), command.c_str());
		}
	} else if (subEntity == SUBENTITY_NAME_TEXT) {
		if (command == COMMAND_SET) {
			displayShowText(payload);
		} else {
			_log->error("Unsupported command: %s %s", subEntity.c_str(), command.c_str());
		}
	} else if (subEntity == SUBENTITY_NAME_SCROLLTEXT) {
		if (command == COMMAND_SET) {
			displayScrollText(payload);
		} else {
			_log->error("Unsupported command: %s %s", subEntity.c_str(), command.c_str());
		}
	} else if (subEntity == SUBENTITY_NAME_BRIGHTNESS) {
		if (command == COMMAND_SET) {
			uint8_t brightness = atoi(payload.c_str());
			displaySetBrightness(brightness);
		} else {
			_log->error("Unsupported command: %s %s", subEntity.c_str(), command.c_str());
		}
	} else {
		_log->error("Unsupported sub entity: %s", subEntity.c_str());
	}
}

void JBWoprMqttDevice::_handleDefconCommand(const std::string& subEntity,
											const std::string& command,
											const std::string& payload) {
	if (subEntity == SUBENTITY_NAME_STATE) {
		if (command == COMMAND_SET) {
			if (payload == STATE_ON) {
				defconLedsSetState(true);
			} else if (payload == STATE_OFF) {
				defconLedsSetState(false);
			} else {
				_log->error("Unsupported payload: %s, %s: %s", subEntity.c_str(), command.c_str(), payload.c_str());
			}
		} else {
			_log->error("Unsupported command: %s %s", subEntity.c_str(), command.c_str());
		}
	} else if (subEntity == SUBENTITY_NAME_LEVEL) {
		if (command == COMMAND_SET) {
			JBDefconLevel level = _getDefconLevel(payload);
			defconLedsSetDefconLevel(level);
		} else {
			_log->error("Unsupported command: %s %s", subEntity.c_str(), command.c_str());
		}
	} else if (subEntity == SUBENTITY_NAME_COLOR) {
		if (command == COMMAND_SET) {
			uint32_t color = JBStringHelper::stringToRgb(payload);
			defconLedsSetColor(color);
		} else {
			_log->error("Unsupported command: %s %s", subEntity.c_str(), command.c_str());
		}
	} else if (subEntity == SUBENTITY_NAME_BRIGHTNESS) {
		if (command == COMMAND_SET) {
			uint8_t brightness = atoi(payload.c_str());
			defconLedsSetBrightness(brightness);
		} else {
			_log->error("Unsupported command: %s %s", subEntity.c_str(), command.c_str());
		}
	} else {
		_log->error("Unsupported sub entity: %s", subEntity.c_str());
	}
}

std::string JBWoprMqttDevice::_getTopic(const char* entityId, const char* subEntityId) {
	// <mqttprefix>/<deviceid>/<entity>/<subentity>
	return _mqttConfig.mqttPrefix + "/" + _getDeviceName() + "/" + entityId + "/" + subEntityId;
}

std::string JBWoprMqttDevice::_getTopic(const std::string& entityId, const std::string& subEntityId) {
	return _getTopic(entityId.c_str(), subEntityId.c_str());
}

std::string JBWoprMqttDevice::_getSubscriptionTopic() {
	// <mqttprefix>/<deviceid>/<entity>/<subentity>/<command>
	return _mqttConfig.mqttPrefix + "/" + _getDeviceName() + "/+/+/+";
}

std::string JBWoprMqttDevice::_getAvailabilityTopic() {
	// <mqttprefix>/<deviceid>/availability"
	return _mqttConfig.mqttPrefix + "/" + _getDeviceName() + "/availability";
}

// ====================================================================
// Buttons
//
void JBWoprMqttDevice::_buttonFrontLeftClick()
{
	JBWoprWiFiDevice::_buttonFrontLeftClick();
	if (_mqttConfig.useMqtt) {
		mqttPublishMessage(_getTopic(ENTITY_NAME_BUTTON_FRONT_LEFT, SUBENTITY_NAME_EVENT), EVENT_CLICK);
	}
}

void JBWoprMqttDevice::_buttonFrontLeftDoubleClick()
{
	JBWoprWiFiDevice::_buttonFrontLeftDoubleClick();
	if (_mqttConfig.useMqtt) {
		mqttPublishMessage(_getTopic(ENTITY_NAME_BUTTON_FRONT_LEFT, SUBENTITY_NAME_EVENT), EVENT_DOUBLE_CLICK);
	}
}

void JBWoprMqttDevice::_buttonFrontRightClick()
{
	JBWoprWiFiDevice::_buttonFrontRightClick();
	if (_mqttConfig.useMqtt) {
		mqttPublishMessage(_getTopic(ENTITY_NAME_BUTTON_FRONT_RIGHT, SUBENTITY_NAME_EVENT), EVENT_CLICK);
	}
}

void JBWoprMqttDevice::_buttonFrontRightDoubleClick()
{
	JBWoprWiFiDevice::_buttonFrontRightDoubleClick();
	if (_mqttConfig.useMqtt) {
		mqttPublishMessage(_getTopic(ENTITY_NAME_BUTTON_FRONT_RIGHT, SUBENTITY_NAME_EVENT), EVENT_DOUBLE_CLICK);
	}
}

void JBWoprMqttDevice::_buttonBackTopClick()
{
	JBWoprWiFiDevice::_buttonBackTopClick();
	if (_mqttConfig.useMqtt) {
		mqttPublishMessage(_getTopic(ENTITY_NAME_BUTTON_BACK_TOP, SUBENTITY_NAME_EVENT), EVENT_CLICK);
	}
}

void JBWoprMqttDevice::_buttonBackTopDoubleClick()
{
	JBWoprWiFiDevice::_buttonBackTopDoubleClick();
	if (_mqttConfig.useMqtt) {
		mqttPublishMessage(_getTopic(ENTITY_NAME_BUTTON_BACK_TOP, SUBENTITY_NAME_EVENT), EVENT_DOUBLE_CLICK);
	}
}

void JBWoprMqttDevice::_buttonBackBottomClick()
{
	JBWoprWiFiDevice::_buttonBackBottomClick();
	if (_mqttConfig.useMqtt) {
		mqttPublishMessage(_getTopic(ENTITY_NAME_BUTTON_BACK_BOTTOM, SUBENTITY_NAME_EVENT), EVENT_CLICK);
	}
}

void JBWoprMqttDevice::_buttonBackBottomDoubleClick()
{
	JBWoprWiFiDevice::_buttonBackBottomDoubleClick();
	if (_mqttConfig.useMqtt) {
		mqttPublishMessage(_getTopic(ENTITY_NAME_BUTTON_BACK_BOTTOM, SUBENTITY_NAME_EVENT), EVENT_DOUBLE_CLICK);
	}
}









