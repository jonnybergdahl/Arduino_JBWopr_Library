/// @file jbwopr.h
/// @author Jonny Bergdahl
/// @brief Main header file for the JBWopr library.
/// @details Contains declarations for the base JBWoprDevice class.
/// This code is distributed under the MIT License. See LICENSE for details.
/// @date Created: 2023-10-02
/// @copyright Copyright© 2023, Jonny Bergdahl
///
#ifndef ARDUINO_JBWOPR_JBWOPR_H
#define ARDUINO_JBWOPR_WOPR_H

#include <Arduino.h>
#include <map>
#include <list>
#include <driver/i2s.h>
#include <jblogger.h>
#include <Adafruit_GFX.h>                  	// https://github.com/adafruit/Adafruit-GFX-Library
#include <Adafruit_LEDBackpack.h>          	// https://github.com/adafruit/Adafruit_LED_Backpack
#include <Adafruit_NeoPixel.h>             	// https://github.com/adafruit/Adafruit_NeoPixel
#include <OneButton.h>                     	// https://github.com/mathertel/OneButton
#include <ArduinoJson.h>					// https://github.com/bblanchon/ArduinoJson
#include "effects/jbwopreffects.h"
#include "jbwoprhelpers.h"

#define LIBRARY_VERSION "1.0.0";

/// @brief W.O.P.R. board version
enum JBWoprBoardVariant {
	ORIGINAL = 0,							///< Original W.O.P.R. board
	HAXORZ                                  ///< Haxorz W.O.P.R. board
};

/// @brief DEFCON levels
enum JBDefconLevel {
	DEFCON_1 = 0,                           ///< DEFCON 1
	DEFCON_2,                               ///< DEFCON 2
	DEFCON_3,                               ///< DEFCON 3
	DEFCON_4,                               ///< DEFCON 4
	DEFCON_5,                               ///< DEFCON 5
	DEFCON_NONE								///< No DEFCON level
};

/// @brief W.O.P.R. board pin assignments
struct JBWoprBoardPins {
	uint8_t buttonFrontLeftPin;             ///< Front left button pin
	uint8_t buttonFrontRightPin;            ///< Front right button pin
	uint8_t buttonBackTopPin;               ///< Back top button pin
	uint8_t buttonBackBottomPin;            ///< Back bottom button pin
	uint8_t defconLedsPin;                  ///< DEFCON LEDs pin
	uint8_t dacPin; 						///< DAC pin
};

/// @brief JBWoprDevice configuration
struct JBWoprConfiguration {
	std::string timeFormat;	                ///< Time format
	std::string dateFormat;                 ///< Date format
	uint8_t displayBrightness;              ///< Display brightness
	uint32_t defconLedsBrightness;          ///< DEFCON LEDs brightness
	uint32_t effectsTimeout;               	///< Effects timeout, seconds
	bool useTimeOnlyClock;                  ///< Time only clock
};

/// @defgroup DisplayGroup Display related methods
/// @defgroup ConfigGroup Configuration related methods
/// @defgroup LoggingGroup Logging related methods
/// @defgroup DefconGroup DEFCON related methods
/// @defgroup AudioGroup Audio related methods
/// @defgroup ButtonGroup Button related methods
/// @defgroup EffectsGroup Effects related methods
/// @defgroup WiFiGroup WiFi related methods
/// @defgroup MqttGroup MQTT related methods
/// @defgroup HAGroup Home Assistant related methods

// ====================================================================
//
// W.O.P.R. base device class
//
// ====================================================================
/// @brief W.O.P.R. device base class
/// @details This class contains all the base functionality for the W.O.P.R.
///
/// This class can be used as is, or as a base class for your own W.O.P.R. device.
/// It exposes convenient methods for controlling the display, DEFCON LEDs, audio and buttons.
/// It also contains a simple effects system that can be used to create your own effects.
///
class JBWoprDevice {
public:
	// ====================================================================
	// General
	//
	/// @brief JBWoprDevice constructor
	JBWoprDevice();

	/// @brief Initialize JBWoprDevice
	/// @ingroup GeneralGroup
	/// @details Use this method if you are using a tinyXxx board, it will
	/// assign the ESP32 pins automatically.
	/// @param variant W.O.P.R. board version
	/// @return True if successful
	virtual bool begin(JBWoprBoardVariant variant = JBWoprBoardVariant::ORIGINAL);

	/// @brief Initialize JBWoprDevice
	/// @ingroup GeneralGroup
	/// @details Use this method if you are using a custom ESP32 board.
	/// @param variant W.O.P.R. board version
	/// @param pins W.O.P.R. ESP32 pin assignments
	/// @return True if successful
	virtual bool begin(JBWoprBoardVariant variant, JBWoprBoardPins pins);

	/// @brief JBWoprDevice loop
	/// @ingroup GeneralGroup
	/// @details This method should be called from the main loop() method.
	virtual void loop();

	/// @brief Get W.O.P.R board variant
	JBWoprBoardVariant getBoardVariant();

	// ====================================================================
	// Configuration
	//
	/// @brief Get current JBWoprDevice configuration
	/// @ingroup ConfigGroup
	/// @return JBWoprDevice configuration
	JBWoprConfiguration* getConfiguration();

	// ====================================================================
	// Logger
	//
	/// @brief Set log level
	/// @ingroup LoggingGroup
	/// @param level Log level
	virtual void setLogLevel(LogLevel level);

	/// @brief Get log level
	/// @ingroup LoggingGroup
	/// @return Log level
	virtual LogLevel getLogLevel();

	// ====================================================================
	// Effects
	//
	/// @brief Register default effect
	/// @note If a default effect is registered, it will be started after
	/// a timeout set by _config.effectsTimeout after any other effect is
	/// done running.
	/// @ingroup EffectsGroup
	/// @param effect Effect to register
	void effectsRegisterDefaultEffect(JBWoprEffectBase* effect);

	/// @brief Check if default effect is running
	/// @ingroup EffectsGroup
	/// @return True if default effect is running
	bool effectsDefaultEffectIsRunning();

	/// @brief Register effect
	/// @ingroup EffectsGroup
	/// @param effect Effect to register
	void effectsRegisterEffect(JBWoprEffectBase* effect);

	/// @brief Get list of registered effects
	/// @ingroup EffectsGroup
	/// @return Default effects
	std::vector<JBWoprEffectBase*> effectsGetRegisteredEffects();

	/// @brief Get current effect
	/// @ingroup EffectsGroup
	/// @return Current effect
	JBWoprEffectBase* effectsGetCurrentEffect();

	/// @brief Start current effect
	/// @ingroup EffectsGroup
	void effectsStartCurrentEffect();

	/// @brief Stop current effect
	/// @ingroup EffectsGroup
	void effectsStopCurrentEffect();

	/// @brief Check if current effect is running
	/// @ingroup EffectsGroup
	/// @return True if current effect is running
	bool effectsCurrentEffectIsRunning();

	/// @brief Start effect
	/// @ingroup EffectsGroup
	/// @param effect Effect to start
	/// @param duration Duration of effect in milliseconds
	virtual void effectsStartEffect(JBWoprEffectBase* effect);

	/// @brief Start named effect
	/// @ingroup EffectsGroup
	/// @param name Name of effect to start
	virtual void effectsStartEffect(std::string name);

	/// @brief Start named effect
	/// @ingroup EffectsGroup
	/// @param name Name of effect to start
	virtual void effectsStartEffect(const char* name);

	// ====================================================================
	// Display
	//
	/// @brief Get display
	/// @ingroup DisplayGroup
	/// @return Display as an array of Adafruit_AlphaNum4 instances
	std::array<Adafruit_AlphaNum4, 3> getDisplay();

	/// @brief Clear display
	/// @ingroup DisplayGroup
	virtual void displayClear();

	/// @brief Force display to show current data
	/// @ingroup DisplayGroup
	virtual void displayShow();

	/// @brief Set display brightness percentage
	/// @ingroup DisplayGroup
	/// @param val Brightness value, 0 - 100
	virtual void displaySetBrightness(uint8_t val);

	/// @brief Set individual display character
	/// @ingroup DisplayGroup
	/// @param index Character index, 0 - 11
	/// @param chr Character to display
	virtual void displaySetChar(uint8_t index, char chr);

	/// @brief Set display text
	/// @ingroup DisplayGroup
	/// @param text Text to display
	/// @param alignment Text alignment
	virtual void displayShowText(const char* text, JBTextAlignment alignment);

	/// @brief Set display text
	/// @ingroup DisplayGroup
	/// @param text Text to display
	/// @param alignment Text alignment
	virtual void displayShowText(const std::string& text, JBTextAlignment alignment);

	/// @brief Set display text
	/// @ingroup DisplayGroup
	/// @param text Text to display
	/// @param alignment Text alignment
	virtual void displayShowText(const String& text, JBTextAlignment alignment);

	/// @brief Set display text, left aligned
	/// @ingroup DisplayGroup
	/// @param text Text to display
	virtual void displayShowText(const char* text);

	/// @brief Set display text, left aligned
	/// @ingroup DisplayGroup
	/// @param text Text to display
	virtual void displayShowText(const std::string& text);

	/// @brief Set display text, left aligned
	/// @ingroup DisplayGroup
	/// @param text Text to display
	virtual void displayShowText(const String& text);

	/// @brief Set display scroll text
	/// @ingroup DisplayGroup
	/// @details This method will block until the text has finished scrolling.
	/// Use the JBWoprScrollTextDisplayEffect effect for a non blocking scroll.
	/// @param text Text to scroll
	/// @param delay_ms Delay between each scroll step
	virtual void displayScrollText(const char* text, uint16_t delay_ms);

	/// @brief Set display scroll text
	/// @ingroup DisplayGroup
	/// @details This method will block until the text has finished scrolling.
	/// Use the JBWoprScrollTextDisplayEffect effect for a non blocking scroll.
	/// @param text Text to scroll
	/// @param delay_ms Delay between each scroll step, default 100 ms
	virtual void displayScrollText(const String& text, uint16_t delay_ms);

	/// @brief Set display scroll text
	/// @ingroup DisplayGroup
	/// @details This method will block until the text has finished scrolling.
	/// Use the JBWoprScrollTextDisplayEffect effect for a non blocking scroll.
	/// @param text Text to scroll
	/// @param delay_ms Delay between each scroll step, default 100 ms
	virtual void displayScrollText(const std::string& text, uint16_t delay_ms);

	/// @brief Set display scroll text with a delay of 100 ms
	/// @ingroup DisplayGroup
	/// @details This method will block until the text has finished scrolling.
	/// Use the JBWoprScrollTextDisplayEffect effect for a non blocking scroll.
	/// @param text Text to scroll
	virtual void displayScrollText(const char* text);

	/// @brief Set display scroll text with a delay of 100 ms
	/// @ingroup DisplayGroup
	/// @details This method will block until the text has finished scrolling.
	/// Use the JBWoprScrollTextDisplayEffect effect for a non blocking scroll.
	/// @param text Text to scroll
	virtual void displayScrollText(const std::string& text);

	/// @brief Set display scroll text with a delay of 100 ms
	/// @ingroup DisplayGroup
	/// @details This method will block until the text has finished scrolling.
	/// Use the JBWoprScrollTextDisplayEffect effect for a non blocking scroll.
	/// @param text Text to scroll
	virtual void displayScrollText(const String& text);

	// ====================================================================
	// DEFCON LEDs
	//
	const char* DEFCON_STRINGS[6] { "DEFCON 1", "DEFCON 2", "DEFCON 3", "DEFCON 4", "DEFCON 5", "None" };

	/// @brief Get DEFCON LEDs
	/// @ingroup DefconGroup
	/// @return Defcon LEDs
	Adafruit_NeoPixel* getDefconLeds();

	/// @brief Set DEFCON level
	/// @ingroup DefconGroup
	/// @param level DEFCON level
	virtual void defconLedsSetDefconLevel(JBDefconLevel level);

	/// @brief Set all DEFCON LED's color
	/// @ingroup DefconGroup
	/// @param color Color value
	virtual void defconLedsSetColor(uint32_t color);

	/// @brief Set DEFCON LED's brightness level
	/// @ingroup DefconGroup
	/// @param brightness Brightness value, 0 - 100
	virtual void defconLedsSetBrightness(uint8_t brightness);

	/// @brief Clear all DEFCON LEDs
	/// @ingroup DefconGroup
	virtual void defconLedsClear();

	// Individual DEFCON LEDs
	/// @brief Set individual DEFCON LED's color
	/// @ingroup DefconGroup
	/// @param level DEFCON level LED
	/// @param color Color value
	virtual void defconLedSetColor(JBDefconLevel level, uint32_t color);

	/// @brief Set color for a DEFCON level
	/// @ingroup DefconGroup
	/// @param level DEFCON level
	/// @param color Color value
	virtual void defconLedSetDefconStateColor(JBDefconLevel level, uint32_t color);

	// ====================================================================
	// Audio
	//
	/// @brief Play audio tone
	/// @ingroup AudioGroup
	/// @param freq Frequency
	void audioPlayTone(const uint16_t freq);

	/// @brief Play audio tone
	/// @ingroup AudioGroup
	/// @param freq Frequency
	/// @param octave Octave
	void audioPlayNote(note_t note, uint8_t octave);

	/// @brief Stop audio playback
	/// @ingroup AudioGroup
	void audioClear();

	// ====================================================================
	// Buttons
	//
	/// @brief Get front left button OneButton instance
	/// @ingroup ButtonGroup
	/// @note Do not use this instance to attach Click or DoubleClick callbacks,
	/// use the buttonFrontLeftSetClickCallback and buttonFrontLeftSetDoubleClickCallback
	/// methods instead.
	/// @return Front left button OneButton instance
	OneButton* getButtonFrontLeft();

	/// @brief Get front right button OneButton instance
	/// @ingroup ButtonGroup
	/// @note Do not use this instance to attach Click or DoubleClick callbacks,
	/// use the buttonFrontRightSetClickCallback and buttonFrontRightSetDoubleClickCallback
	/// methods instead.
	/// @return Front right button OneButton instance
	OneButton* getButtonFrontRight();

	/// @brief Get back top button OneButton instance
	/// @ingroup ButtonGroup
	/// @note Do not use this instance to attach Click or DoubleClick callbacks,
	/// use the buttonBackTopSetClickCallback and buttonBackTopSetDoubleClickCallback
	/// methods instead.
	/// @return Back top button OneButton instance
	OneButton* getButtonBackTop();

	/// @brief Get back bottom button OneButton instance
	/// @ingroup ButtonGroup
	/// @note Do not use this instance to attach Click or DoubleClick callbacks,
	/// use the buttonBackBottomSetClickCallback and buttonBackBottomSetDoubleClickCallback
	/// methods instead.
	/// @return Back bottom button OneButton instance
	OneButton* getButtonBackBottom();

	/// @brief Set front left button click callback
	/// @ingroup ButtonGroup
	/// @param func Callback function
	virtual void buttonFrontLeftSetClickCallback( std::function<void()> func );

	/// @brief Set front left button double click callback
	/// @ingroup ButtonGroup
	/// @param func Callback function
	virtual void buttonFrontLeftSetDoubleClickCallback( std::function<void()> func );

	/// @brief Set front right button click callback
	/// @ingroup ButtonGroup
	/// @param func Callback function
	virtual void buttonFrontRightSetClickCallback( std::function<void()> func );

	/// @brief Set front right button double click callback
	/// @ingroup ButtonGroup
	/// @param func Callback function
	virtual void buttonFrontRightSetDoubleClickCallback( std::function<void()> func );

	/// @brief Set back top button click callback
	/// @ingroup ButtonGroup
	/// @param func Callback function
	virtual void buttonBackTopSetClickCallback( std::function<void()> func );

	/// @brief Set back top button double click callback
	/// @ingroup ButtonGroup
	/// @param func Callback function
	virtual void buttonBackTopSetDoubleClickCallback( std::function<void()> func );

	/// @brief Set back top button long press callback
	/// @ingroup ButtonGroup
	/// @param func Callback function
	virtual void buttonBackBottomSetClickCallback( std::function<void()> func );

	/// @brief Set back bottom button double click callback
	/// @ingroup ButtonGroup
	/// @param func Callback function
	virtual void buttonBackBottomSetDoubleClickCallback( std::function<void()> func );

protected:
	// ====================================================================
	// Configuration
	//
	JBWoprBoardVariant _woprVariant;				///< W.O.P.R. board version
	JBWoprConfiguration _config;					///< JBWoprDevice configuration

	// ====================================================================
	// Effects
	//
	JBWoprEffectBase* _defaultEffect = nullptr;		///< Default effect
	std::vector<JBWoprEffectBase*> _effects;		///< Effects
	JBWoprEffectBase* _currentEffect = nullptr;		///< Current effect
	uint32_t _effectsCounter = 0;					///< Effects counter

	// ====================================================================
	// Display
	//
	std::array<Adafruit_AlphaNum4, 3> _display;		///< Display

	// ====================================================================
	// Defcon LEDs
	//
	JBDefconLevel _getDefconLevel(std::string value);
	std::string _getDefconLevelString(JBDefconLevel level);
	uint32_t _getDefconLedsPixel(JBDefconLevel level);

	Adafruit_NeoPixel _defconLeds = Adafruit_NeoPixel(5, 1, NEO_GRB + NEO_KHZ800);	///< DEFCON LEDs
	uint32_t _defconColors[5];				///< DEFCON colors
	// Note - due to how Adafruit_Neopixel handles brigthness, we buffer the LED colors into the
	// _defconPixels variable together with the current brightness value.
	// We then apply them both before calling show().
	uint32_t _defconPixels[5] { 0,0,0,0,0};	///< DEFCON buffered pixel colors
	uint32_t _defconBrigthtness = 100;		///< DEFCON brightness

	// ====================================================================
	// Buttons
	//
	OneButton* _buttonFrontLeft;			///< Front left button
	OneButton* _buttonFrontRight;			///< Front right button
	OneButton* _buttonBackTop;				///< Back to
	OneButton* _buttonBackBottom;			///< Back bottom button

	std::function<void()> _buttonFrontLeftClickCallback;			///< Front left button click callback
	std::function<void()> _buttonFrontLeftDoubleClickCallback;		///< Front left button double click callback
	std::function<void()> _buttonFrontRightClickCallback;			///< Front right button click callback
	std::function<void()> _buttonFrontRightDoubleClickCallback;		///< Front right button double click callback
	std::function<void()> _buttonBackTopClickCallback;				///< Back top button click callback
	std::function<void()> _buttonBackTopDoubleClickCallback;		///< Back top button double click callback
	std::function<void()> _buttonBackBottomClickCallback;			///< Back bottom button click callback
	std::function<void()> _buttonBackBottomDoubleClickCallback;		///< Back bottom button double click callback

	static void _staticButtonFrontLeftClickCallback(void* data);		///< Front left button internal click callback
	static void _staticButtonFrontLeftDoubleClickCallback(void* data);	///< Front left button internal double click callback
	static void _staticButtonFrontRightClickCallback(void* data);		///< Front right button internal click callback
	static void _staticButtonFrontRightDoubleClickCallback(void* data);	///< Front right button internal double click callback
	static void _staticButtonBackTopClickCallback(void* data);			///< Back top button internal click callback
	static void _staticButtonBackTopDoubleClickCallback(void* data);	///< Back top button internal double click callback
	static void _staticButtonBackBottomClickCallback(void* data);		///< Back bottom button internal click callback
	static void _staticButtonBackBottomDoubleClickCallback(void* data);	///< Back bottom button internal double click callback

	virtual void _buttonFrontLeftClick();								///< Front left button click callback
	virtual void _buttonFrontLeftDoubleClick();							///< Front left button double click callback
	virtual void _buttonFrontRightClick();								///< Front right button click callback
	virtual void _buttonFrontRightDoubleClick();						///< Front right button double click callback
	virtual void _buttonBackTopClick();									///< Back top button click callback
	virtual void _buttonBackTopDoubleClick();							///< Back top button double click callback
	virtual void _buttonBackBottomClick();								///< Back bottom button click callback
	virtual void _buttonBackBottomDoubleClick();						///< Back bottom button double click callback

	// ====================================================================
	// Audio
	bool _useDAC = false;					///< Use DAC
	int _audioFreq = 2000;					///< Audio frequency
	int _audioChannel = 0;					///< Audio channel
	int _audioResolution = 8;				///< Audio resolution

private:
	// ====================================================================
	// Logger
	JBLogger* _log;									///< Logger
};

#endif //ARDUINO_WOPR_WOPR_H
