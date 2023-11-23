/// @file jbwoprmqtt.h
/// @author Jonny Bergdahl
/// @brief Header file for the JBWopr library.
/// @details Contains declarations for the main JBWoprMqttDevice class.
/// This code is distributed under the MIT License. See LICENSE for details.
/// @date Created: 2023-10-25
/// @copyright Copyright© 2023, Jonny Bergdahl
///
#ifndef ARDUINO_WOPR_JBWOPRMQTT_H
#define ARDUINO_WOPR_JBWOPRMQTT_H

#include "jbwoprwifi.h"
#include <PubSubClient.h>				   	// https://github.com/knolleary/pubsubclient
#include <WiFiManager.h>
#include <JBLogger.h>

#define DEFAULT_MQTT_PREFIX	"wopr"			///< Default MQTT prefix
#define DEFAULT_MQTT_PORT 1883				///< Default MQTT port

// ====================================================================
//
// W.O.P.R. device with WiFiManager and MQTT support
//
// ====================================================================
/// @brief JBWoprMqttDevice MQTT configuration
struct JBWoprMqttConfig {
	bool useMqtt;                           ///< Use MQTT
	std::string mqttServerName;             ///< MQTT server name
	uint16_t mqttServerPort;                ///< MQTT server port
	std::string mqttUserName;               ///< MQTT user name
	std::string mqttPassword;               ///< MQTT password
	std::string mqttPrefix;                 ///< MQTT prefix
};

/// @brief W.O.P.R. WiFi and MQTT device class
/// @details This class extends the JBWoprWiFiDevice class with MQTT support.
class JBWoprMqttDevice: public JBWoprWiFiDevice {
public:
	// ====================================================================
	// General
	//
	/// @brief JBWoprMqttDevice constructor
	JBWoprMqttDevice();

	/// @brief Initialize JBWoprMqttDevice
	/// @ingroup GeneralGroup
	/// @details Use this method if you are using a tinyXxx board, it will
	/// assign the ESP32 pins automatically.
	/// @param variant Board version
	/// @return True if successful
	bool begin(JBWoprBoardVariant variant) override;

	/// @brief Initialize JBWoprMqttDevice
	/// @ingroup GeneralGroup
	/// @details Use this method if you are not using a tinyXxx board
	/// @param variant Board version
	/// @param pins Board pins
	/// @return True if successful
	bool begin(JBWoprBoardVariant variant, JBWoprBoardPins pins) override;

	/// @brief JBWoprMqttDevice loop
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
	// MQTT
	//
	/// @brief MQTT publish message
	/// @ingroup MqttGroup
	/// @details This method will publish a message to the MQTT broker.
	/// @param topic MQTT topic
	/// @param jsonDoc JSON document
	/// @param retain Retain message, default value is false
	/// @return True if successful
	bool mqttPublishMessage(const std::string& topic, const DynamicJsonDocument& jsonDoc, bool retain = false);

	/// @brief MQTT publish message
	/// @ingroup MqttGroup
	/// @details This method will publish a message to the MQTT broker.
	/// @param topic MQTT topic
	/// @param value MQTT payload value
	/// @param retain Retain message, default value is false
	/// @return True if successful
	bool mqttPublishMessage(const std::string& topic, const std::string& value, bool retain = false);

	/// @brief MQTT publish message
	/// @ingroup MqttGroup
	/// @details This method will publish a message to the MQTT broker.
	/// @param topic MQTT topic
	/// @param value MQTT payload value
	/// @param retain Retain message, default value is false
	/// @return True if successful
	bool mqttPublishMessage(const char* topic, const char* value, bool retain = false);

	// ====================================================================
	// Display
	//
	/// @brief Display clear
	/// @ingroup DisplayGroup
	/// @details This method will clear the display and publish to
	/// to the MQTT broker.
	void displayClear() override;

	/// @brief Display set brightness
	/// @ingroup DisplayGroup
	/// @details This method will set the display brightness and
	/// publish the brightness to the MQTT broker.
	/// @param val Brightness value
	void displaySetBrightness(uint8_t val) override;

	/// @brief Display show text
	/// @ingroup DisplayGroup
	/// @details This method will show text on the display.
	/// It will also publish the text to the MQTT broker.
	/// @param text Text to show
	/// @param alignment Text alignment, default value is LEFT
	void displayShowText(const char* text, JBTextAlignment alignment) override;

	/// @brief Display show text
	/// @ingroup DisplayGroup
	/// @details This method will show text on the display.
	/// It will also publish the text to the MQTT broker.
	/// @param text Text to show
	/// @param alignment Text alignment, default value is LEFT
	void displayShowText(const std::string& text, JBTextAlignment alignment) override;

	/// @brief Display show text
	/// @ingroup DisplayGroup
	/// @details This method will show text on the display.
	/// It will also publish the text to the MQTT broker.
	/// @param text Text to show
	/// @param alignment Text alignment, default value is LEFT
	void displayShowText(const String& text, JBTextAlignment alignment) override;

	/// @brief Display show text, left aligned
	/// @ingroup DisplayGroup
	/// @details This method will show text on the display.
	/// It will also publish the text to the MQTT broker.
	/// @param text Text to show
	void displayShowText(const char* text) override;

	/// @brief Display show text, left aligned
	/// @ingroup DisplayGroup
	/// @details This method will show text on the display.
	/// It will also publish the text to the MQTT broker.
	/// @param text Text to show
	void displayShowText(const std::string& text) override;

	/// @brief Display show text, left aligned
	/// @ingroup DisplayGroup
	/// @details This method will show text on the display.
	/// It will also publish the text to the MQTT broker.
	/// @param text Text to show
	void displayShowText(const String& text) override;

	/// @brief Display scrolling text
	/// @ingroup DisplayGroup
	/// @details This method will scroll text on the display. It will also publish
	/// the text to the MQTT broker. This method will block until the text has finished
	/// scrolling.
	/// @note Use the JBWoprScrollTextDisplayEffect effect for a non blocking scroll.
	/// @param text Text to scroll
	/// @param delay_ms Delay between scrolls, default value is 100 ms
	void displayScrollText(const char* text, uint16_t delay_ms) override;

	/// @brief Display scrolling text
	/// @ingroup DisplayGroup
	/// @details This method will scroll text on the display. It will also publish
	/// the text to the MQTT broker. This method will block until the text has finished
	/// scrolling.
	/// @note Use the JBWoprScrollTextDisplayEffect effect for a non blocking scroll.
	/// @param text Text to scroll
	/// @param delay_ms Delay between scrolls, default value is 100 ms
	void displayScrollText(const std::string& text, uint16_t delay_ms) override;

	/// @brief Display scrolling text
	/// @ingroup DisplayGroup
	/// @details This method will scroll text on the display. It will also publish
	/// the text to the MQTT broker. This method will block until the text has finished
	/// scrolling.
	/// @note Use the JBWoprScrollTextDisplayEffect effect for a non blocking scroll.
	/// @param text Text to scroll
	/// @param delay_ms Delay between scrolls, default value is 100 ms
	void displayScrollText(const String& text, uint16_t delay_ms) override;

	/// @brief Set display scroll text with a delay of 100 ms
	/// @ingroup DisplayGroup
	/// @details This method will scroll text on the display. It will also publish
	/// the text to the MQTT broker. This method will block until the text has finished
	/// scrolling.
	/// @note Use the JBWoprScrollTextDisplayEffect effect for a non blocking scroll.
	/// @param text Text to scroll
	/// @ingroup DisplayGroup
	virtual void displayScrollText(const char* text);

	/// @brief Set display scroll text with a delay of 100 ms
	/// @ingroup DisplayGroup
	/// @details This method will scroll text on the display. It will also publish
	/// the text to the MQTT broker. This method will block until the text has finished
	/// scrolling.
	/// @note Use the JBWoprScrollTextDisplayEffect effect for a non blocking scroll.
	/// @param text Text to scroll
	/// @ingroup DisplayGroup
	virtual void displayScrollText(const std::string& text);

	/// @brief Set display scroll text with a delay of 100 ms
	/// @ingroup DisplayGroup
	/// @details This method will scroll text on the display. It will also publish
	/// the text to the MQTT broker. This method will block until the text has finished
	/// scrolling.
	/// @note Use the JBWoprScrollTextDisplayEffect effect for a non blocking scroll.
	/// @param text Text to scroll
	/// @ingroup DisplayGroup
	virtual void displayScrollText(const String& text);

	// ====================================================================
	// Defcon
	//
	/// @brief DEFCON LEDs set DEFCON level
	/// @ingroup DefconGroup
	/// @details This method will set the DEFCON level for the DEFCON LEDs.
	/// It will also publish the DEFCON level to the MQTT broker.
	/// @param level DEFCON level
	void defconLedsSetDefconLevel(JBDefconLevel level) override;

	/// @brief DEFCON LEDs set color
	/// @ingroup DefconGroup
	/// @details This method will set the color for the DEFCON LEDs.
	/// It will also publish the color to the MQTT broker.
	/// @param color Color
	void defconLedsSetColor(uint32_t color) override;

	/// @brief DEFCON LEDs set brightness
	/// @ingroup DefconGroup
	/// @details This method will set the brightness for the DEFCON LEDs.
	/// It will also publish the brightness to the MQTT broker.
	/// @param brightness Brightness
	void defconLedsSetBrightness(uint8_t brightness) override;

	/// @brief Set color of a specified DEFCON LED
	/// @ingroup DefconGroup
	/// @details This method will set the color of a specified DEFCON LED.
	/// It will also publish the color to the MQTT broker.
	/// @param level DEFCON level
	/// @param color Color
	void defconLedSetColor(JBDefconLevel level, uint32_t color);

protected:
	// ====================================================================
	// Configuration
	//
	JBWoprMqttConfig _mqttConfig;				///< MQTT configuration

	const char* JSON_KEY_MQTT_USE_MQTT = "useMqtt";					///< Use MQTT key name
	const char* JSON_KEY_MQTT_SERVER_NAME = "mqttServerName";		///< MQTT server name key name
	const char* JSON_KEY_MQTT_SERVER_PORT = "mqttServerPort";		///< MQTT server port key name
	const char* JSON_KEY_MQTT_USER_NAME = "mqttUserName";			///< MQTT user name key name
	const char* JSON_KEY_MQTT_PASSWORD = "mqttPassword";			///< MQTT password key name
	const char* JSON_KEY_CONF_MQTT_PREFIX = "mqttPrefix";				///< MQTT prefix key name

	/// @brief Set JBWoprMqttDevice specific config values from JSON document
	/// @ingroup ConfigurationGroup
	/// @param jsonDoc JSON document
	void _setConfigFromJsonDocument(const DynamicJsonDocument& jsonDoc);

	/// @brief Set JSON document values with JBWoprMqttDevice specific config values
	/// @ingroup ConfigurationGroup
	/// @param jsonDoc JSON document
	void _setJsonDocumentFromConfig(DynamicJsonDocument& jsonDoc);

	// ====================================================================
	// WiFi
	//
	WiFiClient _wifiClient;								///< WiFi client

	const char* HTML_MQTT_TITLE = "<h2>MQTT settings</h2>";			///< MQTT title
	char _mqttServerPortValue[6];									///< MQTT server port value

	// WifiManager parameters
	WiFiManagerParameter* _mqttTitleParam;					///< MQTT title WiFiManager parameter
	WiFiManagerParameter* _useMqttParam;					///< Use MQTT WiFiManager parameter
	WiFiManagerParameter* _mqttServerNameParam;				///< MQTT server name WiFiManager parameter
	WiFiManagerParameter* _mqttServerPortParam;				///< MQTT server port WiFiManager parameter
	WiFiManagerParameter* _mqttUserNameParam;				///< MQTT user name WiFiManager parameter
	WiFiManagerParameter* _mqttPasswordParam;				///< MQTT password WiFiManager parameter
	WiFiManagerParameter* _mqttPrefixParam;					///< MQTT prefix WiFiManager parameter
	WiFiManagerParameter* _break2Param;						///< Break  WiFiManagerparameter

	/// @brief Setup WiFiManager
	/// @ingroup WiFiGroup
	/// @details This method will setup WiFiManager.
	void _setupWiFiManager() override;

	/// @brief Set WiFiManager parameter callback
	/// @ingroup WiFiGroup
	/// @details This method will set the WiFiManager parameter callback.
	/// This will be called when the user clicks Save in the configuration portal.
	void _saveParamsCallback() override;

	// ====================================================================
	// MQTT
	//
	PubSubClient* _mqttClient;						///< MQTT client
	bool _mqttActive = false;						///< MQTT active flag, set tp true after initialization

	const char* ENTITY_NAME_EFFECT = "effect";						///< Effect entity name
	const char* ENTITY_NAME_DISPLAY = "display";					///< Display text entity name
	const char* ENTITY_NAME_DEFCON = "defcon";						///< DEFCON LED entity name
	const char* ENTITY_NAME_BUTTON_FRONT_LEFT = "button_front_left";		///< Button front left entity name
	const char* ENTITY_NAME_BUTTON_FRONT_RIGHT = "button_front_right";	///< Button front right entity name
	const char* ENTITY_NAME_BUTTON_BACK_TOP = "button_back_top";			///< Button back top entity name
	const char* ENTITY_NAME_BUTTON_BACK_BOTTOM = "button_back_bottom";		///< Button back bottom entity name

	const char* SUBENTITY_NAME_STATE = "state";				///< State subentity name
	const char* SUBENTITY_NAME_BRIGHTNESS = "brightness";	///< Brightness subentity name
	const char* SUBENTITY_NAME_COLOR = "color";				///< Color subentity name
	const char* SUBENTITY_NAME_TEXT = "text";				///< Text subentity name
	const char* SUBENTITY_NAME_SCROLLTEXT = "scrolltext";	///< Scroll text subentity name
	const char* SUBENTITY_NAME_EVENT = "event";				///< Event subentity name
	const char* SUBENTITY_NAME_LEVEL = "level";				///< Level subentity name
	const char* SUBENTITY_NAME_NAME = "name";				///< Effect subentity name

	const char* STATE_ON = "ON";							///< State ON
	const char* STATE_OFF = "OFF";							///< State OFF

	const char* EVENT_CLICK = "click";						///< Click event
	const char* EVENT_DOUBLE_CLICK = "double_click";		///< Double click event

	const char* COMMAND_SET = "set";							///< Set command

	/// @brief Start MQTT
	/// @ingroup MqttGroup
	/// @details This method will initialize the MQTT client.
	/// @return True if successful
	bool _mqttStart();

	/// @brief Stop MQTT
	/// @ingroup MqttGroup
	/// @details This method will stop the MQTT client.
	void _mqttStop();

	/// @brief Reconnect MQTT
	/// @ingroup MqttGroup
	/// @details This method will check if connection is lost and try to reconnect
	/// the MQTT client as needed.
	bool _mqttReconnect();

	/// @brief Called when MQTT client get connected
	/// @ingroup MqttGroup
	/// @details This method will be called when the MQTT client is connected.
	virtual bool _onMqttConnect();

	/// @brief MQTT callback
	/// @ingroup MqttGroup
	/// @details This method is the callback for the MQTT client, and will
	/// handle incoming messages.
	/// @param topic MQTT topic
	/// @param payload MQTT payload
	/// @param length MQTT payload length
	void _mqttCallback(const char* topic, const byte* payload, unsigned int length);

	/// @brief Handle MQTT command message
	/// @ingroup MqttGroup
	/// @details This method will handle incoming MQTT command messages.
	/// @param entity Entity name
	/// @param subEntity Sub entity name
	/// @param command Command
	/// @param payload Payload
	virtual void _handleCommand(const std::string& entity, const std::string& subEntity, const std::string& command, const std::string& payload);

	/// @brief Handle MQTT effect command message
	/// @ingroup MqttGroup
	/// @details This method will handle incoming MQTT effect command messages.
	/// @param subEntity Sub entity name
	/// @param command Command
	/// @param payload Payload
	virtual void _handleEffectCommand(const std::string& subEntity, const std::string& command, const std::string& payload);

	/// @brief Handle MQTT display command message
	/// @ingroup MqttGroup
	/// @details This method will handle incoming MQTT display command messages.
	/// @param subEntity Sub entity name
	/// @param command Command
	/// @param payload Payload
	virtual void _handleDisplayCommand(const std::string& subEntity, const std::string& command, const std::string& payload);

	/// @brief Handle MQTT defcon command message
	/// @ingroup MqttGroup
	/// @details This method will handle incoming MQTT defcon command messages.
	/// @param subEntity Sub entity name
	/// @param command Command
	/// @param payload Payload
	virtual void _handleDefconCommand(const std::string& subEntity, const std::string& command, const std::string& payload);

	/// @brief Get publish topic for specified entity
	/// @ingroup MqttGroup
	/// @param entityId Entity ID
	/// @param subEntityId Sub entity ID
	/// @return Topic
	virtual std::string _getTopic(const char * entityId, const char* subEntityId);

	/// @brief Get publish topic for specified entity
	/// @ingroup MqttGroup
	/// @param entityId Entity ID
	/// @param subEntityId Sub entity ID
	/// @return Topic
	virtual std::string _getTopic(const std::string& entityId, const std::string& subEntityId);

	/// @brief Get subscription topic
	/// @ingroup MqttGroup
	/// @return Subscription topic
	virtual std::string _getSubscriptionTopic();

	/// @brief Get availability topic
	/// @ingroup MqttGroup
	/// @return Availability topic
	virtual std::string _getAvailabilityTopic();

	// ====================================================================
	// Buttons
	//
	/// @brief Button front left click callback
	/// @ingroup ButtonGroup
	void _buttonFrontLeftClick() override;

	/// @brief Button front left double click callback
	/// @ingroup ButtonGroup
	void _buttonFrontLeftDoubleClick() override;

	/// @brief Button front right click callback
	/// @ingroup ButtonGroup
	void _buttonFrontRightClick() override;

	/// @brief Button front right double click callback
	/// @ingroup ButtonGroup
	void _buttonFrontRightDoubleClick() override;

	/// @brief Button back top click callback
	/// @ingroup ButtonGroup
	void _buttonBackTopClick() override;

	/// @brief Button back top double click callback
	/// @ingroup ButtonGroup
	void _buttonBackTopDoubleClick() override;

	/// @brief Button back bottom click callback
	/// @ingroup ButtonGroup
	void _buttonBackBottomClick() override;

	/// @brief Button back bottom double click callback
	/// @ingroup ButtonGroup
	void _buttonBackBottomDoubleClick() override;

private:
	// ====================================================================
	// Logger
	//
	JBLogger* _log;									///< Logger

};

#endif //ARDUINO_WOPR_JBWOPRMQTT_H
