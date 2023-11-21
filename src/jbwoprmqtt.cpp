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
	if (_mqttConfig.useMqtt) {
		_log->info("Starting MQTT");
		displayShowText("Start MQTT");
		defconLedsSetColor(0x0000FF);

		if (!_mqttStart()) {
			_log->error("Failed to start MQTT");
			return false;
		}
	}
	return true;
}

void JBWoprMqttDevice::loop() {
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
bool JBWoprMqttDevice::mqttPublishMessage(const std::string configTopic, const DynamicJsonDocument& jsonDoc, bool retain) {
	char payload[1024];
	serializeJson(jsonDoc, payload);

	return mqttPublishMessage(configTopic.c_str(), payload, retain);
}

bool JBWoprMqttDevice::mqttPublishMessage(const std::string topic, const std::string payload, bool retain) {

	return mqttPublishMessage(topic.c_str(), payload.c_str(), retain);
}

bool JBWoprMqttDevice::mqttPublishMessage(const char* configTopic, const char* value, bool retain) {
	if (!_mqttActive) {
		_log->trace("MQTT not active, skipping publish");
		return false;
	}

	_log->trace("MQTT Publish %s %s:", configTopic, retain ? "(retain)" : "");
	_log->traceAsciiDump(value, strlen(value));

	if (!_mqttClient->connected()) {
		_log->trace("MQTT not connected, skipping publish");
		return false;
	}
	if (!_mqttClient->publish(configTopic, value, retain))
	{
		_log->error("Failed to publish to MQTT topic");
		return false;
	}
	return true;
}

// ====================================================================
// Display
//
void JBWoprMqttDevice::displayClear() {
	JBWoprWiFiDevice::displayClear();
	mqttPublishMessage(_getTopic(ENTITY_NAME_DISPLAY, SUBENTITY_NAME_TEXT), "");
}

void JBWoprMqttDevice::displaySetBrightness(uint8_t val) {
	JBWoprWiFiDevice::displaySetBrightness(val);
	mqttPublishMessage(_getTopic(ENTITY_NAME_DISPLAY, SUBENTITY_NAME_BRIGHTNESS), val);
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
void JBWoprMqttDevice::defconLedsSetDefconLevel(JBDefconLevel level) {
	JBWoprWiFiDevice::defconLedsSetDefconLevel(level);
	mqttPublishMessage(_getTopic(ENTITY_NAME_DEFCON, SUBENTITY_NAME_LEVEL), DEFCON_STRINGS[level]);
}
void JBWoprMqttDevice::defconLedsSetColor(uint32_t color) {
	JBWoprWiFiDevice::defconLedsSetColor(color);
}
void JBWoprMqttDevice::defconLedsSetBrightness(uint8_t brightness) {
	JBWoprWiFiDevice::defconLedsSetBrightness(brightness);
}

void JBWoprMqttDevice::defconLedSetColor(JBDefconLevel level, uint32_t color) {
	JBWoprWiFiDevice::defconLedSetColor(level, color);
	mqttPublishMessage(_getTopic(ENTITY_NAME_DEFCON, SUBENTITY_NAME_COLOR), color);
}

// ====================================================================
// Configuration
//
void JBWoprMqttDevice::_setConfigFromJsonDocument(const DynamicJsonDocument& jsonDoc) {
	JBWoprWiFiDevice::_setConfigFromJsonDocument(jsonDoc);
	if (!jsonDoc[CONF_MQTT_USE_MQTT_KEY].isNull()) {
		_mqttConfig.useMqtt = jsonDoc[CONF_MQTT_USE_MQTT_KEY].as<bool>();
	}
	if (!jsonDoc[CONF_MQTT_SERVER_NAME_KEY].isNull()) {
		_mqttConfig.mqttServerName = jsonDoc[CONF_MQTT_SERVER_NAME_KEY].as<std::string>();
	}
	if (!jsonDoc[CONF_MQTT_SERVER_PORT_KEY].isNull()) {
		_mqttConfig.mqttServerPort = jsonDoc[CONF_MQTT_SERVER_PORT_KEY].as<uint16_t>();
	}
	if (!jsonDoc[CONF_MQTT_USER_NAME_KEY].isNull()) {
		_mqttConfig.mqttUserName = jsonDoc[CONF_MQTT_USER_NAME_KEY].as<std::string>();
	}
	if (!jsonDoc[CONF_MQTT_PASSWORD_KEY].isNull()) {
		_mqttConfig.mqttPassword = jsonDoc[CONF_MQTT_PASSWORD_KEY].as<std::string>();
	}
	if (!jsonDoc[CONF_MQTT_PREFIX_KEY].isNull()) {
		_mqttConfig.mqttPrefix = jsonDoc[CONF_MQTT_PREFIX_KEY].as<std::string>();
	}
}

void JBWoprMqttDevice::_setJsonDocumentFromConfig(DynamicJsonDocument& jsonDoc) {
	JBWoprWiFiDevice::_setJsonDocumentFromConfig(jsonDoc);
	jsonDoc[CONF_MQTT_USE_MQTT_KEY] = _mqttConfig.useMqtt;
	jsonDoc[CONF_MQTT_SERVER_NAME_KEY] = _mqttConfig.mqttServerName;
	jsonDoc[CONF_MQTT_SERVER_PORT_KEY] = _mqttConfig.mqttServerPort;
	jsonDoc[CONF_MQTT_USER_NAME_KEY] = _mqttConfig.mqttUserName;
	jsonDoc[CONF_MQTT_PASSWORD_KEY] = _mqttConfig.mqttPassword;
	jsonDoc[CONF_MQTT_PREFIX_KEY] = _mqttConfig.mqttPrefix;
}

// ====================================================================
// WiFi
//
void JBWoprMqttDevice::_setupWiFiManager(){
	JBWoprWiFiDevice::_setupWiFiManager();

	auto wifiManager = _getWiFiManager();
	snprintf(_mqttServerPortValue, sizeof(_mqttServerPortValue), "%d", _mqttConfig.mqttServerPort);

	_mqttTitleParam = new WiFiManagerParameter(HTML_MQTT_TITLE);
	_break2Param = new WiFiManagerParameter("<br/>");
	_useMqttParam = new WiFiManagerParameter(CONF_MQTT_USE_MQTT_KEY, "Use MQTT", "T", 2, _mqttConfig.useMqtt ? HTML_CHECKBOX_TRUE : HTML_CHECKBOX_FALSE, WFM_LABEL_AFTER);
	_mqttServerNameParam = new WiFiManagerParameter(CONF_MQTT_SERVER_NAME_KEY, "MQTT server", _mqttConfig.mqttServerName.c_str(), 40);
	_mqttServerPortParam = new WiFiManagerParameter(CONF_MQTT_SERVER_PORT_KEY, "MQTT port", _mqttServerPortValue, 5);
	_mqttUserNameParam = new WiFiManagerParameter(CONF_MQTT_USER_NAME_KEY, "MQTT user name", _mqttConfig.mqttUserName.c_str(), 40);
	_mqttPasswordParam = new WiFiManagerParameter(CONF_MQTT_PASSWORD_KEY, "MQTT password", _mqttConfig.mqttPassword.c_str(), 40);
	_mqttPrefixParam = new WiFiManagerParameter(CONF_MQTT_PREFIX_KEY, "MQTT prefix", _mqttConfig.mqttPrefix.c_str(), 40);

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
	_mqttConfig.mqttPrefix == std::string(_mqttPrefixParam->getValue());
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
	std::string lastWillTopic("wopr/WOPR-471da0d8/availability");
	std::string lastWillPayload("offline");

	for (int retry = 0; retry < maxRetries; ++retry) {
		if (!_mqttClient->connect(_getDeviceName().c_str(),
								  _mqttConfig.mqttUserName.c_str(),
								  _mqttConfig.mqttPassword.c_str(),
								  lastWillTopic.c_str(),
								  1,
								  true,
								  lastWillPayload.c_str())) {
			_log->error("Failed to connect to MQTT server, error %i", _mqttClient->state());
			delay(1000);
		} else {
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
	return true;
}

void JBWoprMqttDevice::_mqttCallback(const char* topic, const byte* payload, unsigned int length){
	_log->trace("MQTT callback");
	_log->trace("Topic: %s", topic);
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
	if (entity == ENTITY_NAME_EFFECT) {
		_handleEffectCommand(subEntity, command, payload);
	} else if (entity == ENTITY_NAME_DISPLAY) {
		_handleDisplayCommand(subEntity, command, payload);
	} else if (entity == ENTITY_NAME_DEFCON) {
		_handleDefconCommand(subEntity, command, payload);
	} else {
		_log->error("Unsupported entity: %s", entity.c_str());
	}
}

void JBWoprMqttDevice::_handleEffectCommand(const std::string& subEntity, const std::string& command, const std::string& payload) {
	if (subEntity == SUBENTITY_NAME_NAME) {
		if (command == COMMAND_SET) {
			effectsStartEffect(payload.c_str());
		}
	} else if (subEntity == SUBENTITY_NAME_STATE) {
		if (command == COMMAND_SET) {
			if (payload == STATE_ON) {
				effectsStartCurrentEffect();
			} else if (payload == STATE_OFF) {
				effectsStopCurrentEffect();
			}
		}
	} else {
		_log->error("Unsupported sub entity: %s", subEntity.c_str());
	}
}

void JBWoprMqttDevice::_handleDisplayCommand(const std::string& subEntity, const std::string& command, const std::string& payload) {
	if (subEntity == SUBENTITY_NAME_TEXT) {
		if (command == COMMAND_SET) {
			displayShowText(payload);
		}
	} else if (subEntity == SUBENTITY_NAME_BRIGHTNESS) {
		if (command == COMMAND_SET) {
			uint8_t brightness = atoi(payload.c_str());
			displaySetBrightness(brightness);
		}
	} else if (subEntity == SUBENTITY_NAME_SCROLLTEXT) {
		if (command == COMMAND_SET) {
			displayScrollText(payload);
		}
	} else {
		_log->error("Unsupported sub entity: %s", subEntity.c_str());
	}
}

void JBWoprMqttDevice::_handleDefconCommand(const std::string& subEntity, const std::string& command, const std::string& payload) {
	if (subEntity == SUBENTITY_NAME_LEVEL) {
		if (command == COMMAND_SET) {
			JBDefconLevel level = _getDefconLevel(payload);
			defconLedsSetDefconLevel(level);
		}
	} else if (subEntity == SUBENTITY_NAME_COLOR) {
		if (command == COMMAND_SET) {
			uint32_t color = JBStringHelper::stringToRgb(payload);
			defconLedsSetColor(color);
		}
	} else if (subEntity == SUBENTITY_NAME_BRIGHTNESS) {
		if (command == COMMAND_SET) {
			uint8_t brightness = atoi(payload.c_str());
			defconLedsSetBrightness(brightness);
		}
	} else {
		_log->error("Unsupported sub entity: %s", subEntity.c_str());
	}
}

std::string JBWoprMqttDevice::_getTopic(const char* entityId, const char* subEntityId) {
	return _mqttConfig.mqttPrefix + "/" + _getDeviceName() + "/" + entityId + "/" + subEntityId;
}

std::string JBWoprMqttDevice::_getSubscriptionTopic() {
	return _mqttConfig.mqttPrefix + "/" + _getDeviceName() + "/#";
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







