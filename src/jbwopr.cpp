/// @file jbwopr.cpp
/// @author Jonny Bergdahl
/// @brief Main source file the JBWopr library.
/// Github: https://github.com/jonnybergdahl/Arduino_JBWopr_Library
/// @date Created: 2023-10-02
/// @details This code is distributed under the MIT License. See LICENSE for details.
#include "jbwopr.h"
#include "jbwoprhelpers.h"
#include <string>
#include <esp_system.h>
#include <WiFi.h>

// ====================================================================
// General
//
JBWoprDevice::JBWoprDevice() :
	_config {
		"%H %M %S",				// separatorCharacter
		"%Y-%m-%d",				// timeGmtOffset
		50,					// displayBrightness
		50,				// defconLedsBrightness
		30					// effectsTimeout
	},
   _display { Adafruit_AlphaNum4(), Adafruit_AlphaNum4(), Adafruit_AlphaNum4() },
   _defconColors { 0xFFFFFF, 0xFF0000, 0xFFFF00, 0x00FF00, 0x0000FF }
{
	_log = new JBLogger("wopr", LogLevel::LOG_LEVEL_INFO);
};

bool JBWoprDevice::begin(JBWoprBoardVariant variant) {
#if defined(ARDUINO_TINYS2)    // Setup pin assignments based on MCU, assuming a tinyXxx variant
	JBWoprBoardPins pinAssignments = {
		.buttonFrontLeftPin = 6,
		.buttonFrontRightPin = 5,
		.buttonBackTopPin = 38,
		.buttonBackBottomPin = 33,
		.defconLedsPin = 7,
		.dacPin = 18,
};
#elif defined(ARDUINO_TINYS3)
	JBWoprBoardPins pinAssignments = {
		.buttonFrontLeftPin = 3,
		.buttonFrontRightPin = 2,
		.buttonBackTopPin = 7,
		.buttonBackBottomPin = 6,
		.defconLedsPin = 4,
		.dacPin = 21,
};
#elif defined(ARDUINO_TINYPICO)
	JBWoprBoardPins pinAssignments = {
			.buttonFrontLeftPin = 15,
			.buttonFrontRightPin = 14,
			.buttonBackTopPin = 32,
			.buttonBackBottomPin = 33,
			.defconLedsPin = 27,
			.dacPin = 25
	};
#else
	JBWoprBoardPins pinAssignments = {};
#error "No tiny board variant detected, need to use the begin(variant, pins) method variant to specify pin assignments"
#endif
	return begin(variant, pinAssignments);
}

bool JBWoprDevice::begin(JBWoprBoardVariant variant, JBWoprBoardPins pins) {
	_woprVariant = variant;
	_log->info("JBWoprDevice begin, variant: %i", variant);

	_pins = pins;
	JBTimeHelper::configure(_log);

	// Buttons
	_log->trace("Button pins: %i, %i, %i, %i", pins.buttonFrontLeftPin, pins.buttonFrontRightPin, pins.buttonBackTopPin, pins.buttonBackBottomPin);
	_buttonFrontLeft = new OneButton(pins.buttonFrontLeftPin, false);
	_buttonFrontRight = new OneButton(pins.buttonFrontRightPin, false);
	if (variant == JBWoprBoardVariant::HAXORZ) {
		_buttonBackTop = new OneButton(pins.buttonBackTopPin, false);
		_buttonBackBottom = new OneButton(pins.buttonBackBottomPin, false);
	}
	// Display
	if (!_display[0].begin(0x70)) {
		_log->error("Display 0 not found");
		return false;
	}
	if (!_display[1].begin(0x72)) {
		_log->error("Display 1 not found");
		return false;
	}
	if (!_display[2].begin(0x74)) {
		_log->error("Display 2 not found");
		return false;
	}
	displaySetBrightness(_config.displayBrightness);
	displayClear();
	delay(1000);

	// DEFCON LEDs
	_defconLeds.setPin(pins.defconLedsPin);
	_defconLeds.begin();
	_defconLeds.setBrightness(_config.defconLedsBrightness);
	_defconLeds.clear();
	_defconLeds.show();

	// Audio
	pinMode(pins.dacPin, OUTPUT);
#if ESP_ARDUINO_VERSION_MAJOR < 3
	if (ledcSetup(_audioChannel, _audioFreq, _audioResolution) == 0) {
		_log->error("Audio setup failed");
		return false;
	};
	ledcAttachPin(pins.dacPin, _audioChannel);
#else
	ledcAttachChannel(pins.dacPin, _audioFreq, _audioResolution, _audioChannel);
#endif
	// Buttons
	_buttonFrontLeft->attachClick(&JBWoprDevice::_staticButtonFrontLeftClickCallback, this);
	_buttonFrontLeft->attachDoubleClick(&JBWoprDevice::_staticButtonFrontLeftDoubleClickCallback, this);
	_buttonFrontRight->attachClick(&JBWoprDevice::_staticButtonFrontRightClickCallback, this);
	_buttonFrontRight->attachDoubleClick(&JBWoprDevice::_staticButtonFrontRightDoubleClickCallback, this);
	if (_woprVariant == JBWoprBoardVariant::HAXORZ) {
		_buttonBackTop->attachClick(&JBWoprDevice::_staticButtonBackTopClickCallback, this);
		_buttonBackTop->attachDoubleClick(&JBWoprDevice::_staticButtonBackTopDoubleClickCallback, this);
		_buttonBackBottom->attachClick(&JBWoprDevice::_staticButtonBackBottomClickCallback, this);
		_buttonBackBottom->attachDoubleClick(&JBWoprDevice::_staticButtonBackBottomDoubleClickCallback, this);
	}

	return true;
}

void JBWoprDevice::loop()
{
	_buttonFrontLeft->tick();
	_buttonFrontRight->tick();
	if (_woprVariant == JBWoprBoardVariant::HAXORZ) {
		_buttonBackTop->tick();
		_buttonBackBottom->tick();
	}

	// Handle current effect
	if (effectsCurrentEffectIsRunning()) {
		_currentEffect->loop();
		return;
	}

	if (_defaultEffect == nullptr) {
		return;
	}

	if (!effectsDefaultEffectIsRunning()) {

		if (_effectsCounter == 0) {
			// Effect just stopped running, start timer
			_effectsCounter = millis() + _config.effectsTimeout * 1000;
			return;
		}

		if (millis() > _effectsCounter) {
			// Timer expired, start default effect
			effectsStartEffect(_defaultEffect);
			_effectsCounter = 0;
			return;
		}
	}

	if (_defaultEffect->isRunning()) {
		_defaultEffect->loop();
	}
}

JBWoprConfiguration* JBWoprDevice::getConfiguration() {
	return &_config;
}

JBWoprBoardVariant JBWoprDevice::getBoardVariant() {
	return _woprVariant;
}

// ====================================================================
// Effects
//
void JBWoprDevice::effectsRegisterDefaultEffect(JBWoprEffectBase* effect) {
	_defaultEffect = effect;
}

bool JBWoprDevice::effectsDefaultEffectIsRunning() {
	if (_defaultEffect != nullptr) {
		return _defaultEffect->isRunning();
	}
	return false;
}

void JBWoprDevice::effectsRegisterEffect(JBWoprEffectBase* effect) {
	_effects.push_back(effect);
}

std::vector<JBWoprEffectBase*> JBWoprDevice::effectsGetRegisteredEffects() {
	return _effects;
}

JBWoprEffectBase* JBWoprDevice::effectsGetCurrentEffect() {
	return _currentEffect;
}

void JBWoprDevice::effectsStartCurrentEffect() {
	_effectsCounter = 0;
	if (_currentEffect != nullptr) {
		_currentEffect->start();
	}
}

void JBWoprDevice::effectsStopCurrentEffect() {
	if (_currentEffect != nullptr) {
		_currentEffect->stop();
	}
}

bool JBWoprDevice::effectsCurrentEffectIsRunning() {
	if (_currentEffect != nullptr) {
		return _currentEffect->isRunning();
	}
	return false;
}

void JBWoprDevice::effectsStartEffect(JBWoprEffectBase* effect) {
	_log->trace("Starting effect %s", effect->getName().c_str());
	_effectsCounter = 0;
	_currentEffect = effect;
	_currentEffect->start();
}

void JBWoprDevice::effectsStartEffect(const std::string& name) {
	for (const auto& effect : _effects) {
		if (effect->getName() == name) {
			effectsStartEffect(effect);
			return;
		}
	}
}

void JBWoprDevice::effectsStartEffect(const char* name) {
	effectsStartEffect(std::string(name));
}

// ------------------------------------------------------------------
//
// Display related methods
//
// ------------------------------------------------------------------

std::array<Adafruit_AlphaNum4, 3> JBWoprDevice::getDisplay() {
	return _display;
}

void JBWoprDevice::displaySetState(bool state) {
	_displayState = state;
	for (int x = 0; x < 3; x++) {
		_display[x].setDisplayState(state);
	}
}

void JBWoprDevice::displaySetBrightness(uint8_t value)
{
	value = constrain(value, 0, 100);
	_displayBrightness = map(value, 0, 100, 0, 15);;
	for (int x = 0; x < 3; x++) {
		_display[x].setBrightness(_displayState ? _displayBrightness : 0);
	}
}

void JBWoprDevice::displayClear()
{
	for ( int i = 0; i < 3; i++ ) {
		for (int d = 0; d < 4; d++) {
			_display[i].writeDigitAscii(d, ' ');
		}
	}
	displayShow();
}

void JBWoprDevice::displayShow()
{
	_display[0].writeDisplay();
	_display[1].writeDisplay();
	_display[2].writeDisplay();
}

void JBWoprDevice::displaySetChar(uint8_t index, char chr)
{
	int displayIndex = index / 4;
	int digitIndex = index % 4;
	_display[displayIndex].writeDigitAscii(digitIndex, chr);
}

void JBWoprDevice::displayShowText(const char* text, JBTextAlignment alignment)
{
	uint8_t curDisplay = 0;
	uint8_t curDigit = 0;
	std::string displayText = text;
	size_t textLength = displayText.length();
	uint32_t padSize = (12 - textLength) / 2;
	uint32_t startIndex = 0;
	switch (alignment) {
		case JBTextAlignment::LEFT:
			startIndex = 0;
			break;
		case JBTextAlignment::RIGHT:
			startIndex = 12 - textLength;
			break;
		case JBTextAlignment::CENTER:
			startIndex = padSize;
			break;
	}
	uint32_t endIndex = startIndex + textLength;

	for ( uint8_t i = 0; i < 12; i++ )
	{
		if (i < startIndex || i >= endIndex) {
			_display[curDisplay].writeDigitAscii(curDigit, ' ');
		} else {
			_display[curDisplay].writeDigitAscii(curDigit, displayText.at(i - startIndex));
		}
		curDigit++;
		if (curDigit == 4)
		{
			curDigit = 0;
			curDisplay++;
		}
	}

	displayShow();
}

void JBWoprDevice::displayShowText(const std::string& text, JBTextAlignment alignment)
{
	displayShowText(text.c_str(), alignment);
}

void JBWoprDevice::displayShowText(const String& text, JBTextAlignment alignment)
{
	displayShowText(text.c_str(), alignment);
}

void JBWoprDevice::displayShowText(const char* text) {
	displayShowText(text, JBTextAlignment::LEFT);
}

void JBWoprDevice::displayShowText(const std::string& text)
{
	displayShowText(text.c_str(), JBTextAlignment::LEFT);
}

void JBWoprDevice::displayShowText(const String& text)
{
	displayShowText(text.c_str(), JBTextAlignment::LEFT);
}

void JBWoprDevice::displayScrollText(const char* text, uint16_t delay_ms) {
	uint8_t curDisplay = 0;
	uint8_t curDigit = 0;
	std::string displayText = text;
	size_t textLength = displayText.length();
	size_t totalLength = textLength + 24;
	size_t startIndex = 12;
	size_t endIndex = startIndex + textLength;

	for (size_t i = 0; i < totalLength; i++)
	{
		curDisplay = 0;
		curDigit = 0;
		uint8_t crap = 0;
		for (uint8_t j = 0; j < 12; j++ ) {
			if (i + j < startIndex || i + j >= endIndex) {
				_display[curDisplay].writeDigitAscii(curDigit, ' ');
				if (i + j >= endIndex) {
					crap++;
				}

			} else {
				_display[curDisplay].writeDigitAscii(curDigit, displayText.at(i - startIndex + j));
			}
			curDigit++;
			if (curDigit == 4) {
				curDigit = 0;
				curDisplay++;
			}
		}
		displayShow();
		delay(delay_ms);
	}
}

void JBWoprDevice::displayScrollText(const std::string& text, uint16_t delay_ms) {
	displayScrollText(text.c_str(), delay_ms);
}

void JBWoprDevice::displayScrollText(const String& text, uint16_t delay_ms) {
	displayScrollText(text.c_str(), delay_ms);
}

void JBWoprDevice::displayScrollText(const char* text) {
	displayScrollText(text, 100);
}

void JBWoprDevice::displayScrollText(const std::string& text) {
	displayScrollText(text.c_str(), 100);
}

void JBWoprDevice::displayScrollText(const String& text) {
	displayScrollText(text.c_str(), 100);
}

// ------------------------------------------------------------------
//
// DEFCON Led's related methods
//
// ------------------------------------------------------------------

Adafruit_NeoPixel* JBWoprDevice::getDefconLeds() {
	return &_defconLeds;
}

void JBWoprDevice::defconLedsSetState(bool state) {
	_defconState = state;
	_defconLeds.setBrightness(state ? _defconBrigthtness : 0);
	for (int i = 0; i < 5; i++) {
		_defconLeds.setPixelColor(i, _defconPixels[i]);
	}
	_defconLeds.show();
}

void JBWoprDevice::defconLedsSetDefconLevel(JBDefconLevel level) {
	_log->trace("defconLedsSetDefconLevel %s", _getDefconLevelString(level).c_str());
	_defconLevel = level;
	uint32_t led = _getDefconLedsPixel(level);
	_defconLeds.setBrightness(_defconState ? _defconBrigthtness : 0);
	for (uint32_t i = 0; i < 5; i++) {
		if (i == led) {
			_defconPixels[i] = _defconColors[int(level)];
			_defconLeds.setPixelColor(i, _defconColors[int(level)]);
		}
		else {
			_defconPixels[i] = 0;
			_defconLeds.setPixelColor(i, 0);
		}
	}
	_defconLeds.show();
}

void JBWoprDevice::defconLedsSetColor(uint32_t color)
{
	_log->trace("defconLedsSetColor %s", JBStringHelper::rgbToString(color).c_str());
	_defconLedsColor = color;
	_defconLeds.setBrightness(_defconState ? _defconBrigthtness : 0);
	for (int i = 0; i < 5; i++) {
		_defconPixels[i] = color;
		_defconLeds.setPixelColor(i, color);
	}
	_defconLeds.show();
}

void JBWoprDevice::defconLedsSetBrightness(uint8_t brightness) {
	_log->trace("defconLedsSetBrightness %i", brightness);
	brightness = constrain(brightness, 0, 100);
 	_defconBrightness = map(brightness, 0, 100, 0, 255);
	_defconLeds.setBrightness(_defconState ? _defconBrightness : 0);
	for (int i = 0; i < 5; i++) {
		_defconLeds.setPixelColor(i, _defconPixels[i]);
	}
	_defconLeds.show();
}

void JBWoprDevice::defconLedsClear()
{
	_log->trace("defconLedsClear");
	_defconLeds.clear();
	_defconLeds.show();
}

void JBWoprDevice::defconLedSetColor(JBDefconLevel level, uint32_t color)
{
	_log->trace("defconLedSetColor %s, %s", _getDefconLevelString(level).c_str(), JBStringHelper::rgbToString(color).c_str());
	if (level != JBDefconLevel::DEFCON_NONE) {
		uint32_t pixel = _getDefconLedsPixel(level);
		_defconPixels[(int) level] = color;
		_defconLeds.setPixelColor(pixel, color);
		_defconLeds.show();
	}
}

void JBWoprDevice::defconLedSetDefconStateColor(JBDefconLevel level, uint32_t color)
{
	_log->trace("defconLedSetDefconStateColor %s, %s", _getDefconLevelString(level).c_str(), JBStringHelper::rgbToString(color).c_str());
	if (level != JBDefconLevel::DEFCON_NONE) {
		uint32_t pixel = _getDefconLedsPixel(level);
		_defconColors[pixel] = color;
	}
}

// ------------------------------------------------------------------
//
// Audio related methods
//
// ------------------------------------------------------------------

void JBWoprDevice::audioPlayTone(uint16_t freq)
{
#if ESP_ARDUINO_VERSION_MAJOR < 3
	ledcWriteTone(_audioChannel, freq);
#else
	ledcWriteTone(_pins.dacPin, freq);
#endif
}

void JBWoprDevice::audioPlayNote(note_t note, uint8_t octave)
{
#if ESP_ARDUINO_VERSION_MAJOR < 3
	ledcWriteNote(_audioChannel, note, octave);
#else
	ledcWriteNote(_pins.dacPin, note, octave);
#endif
}

void JBWoprDevice::audioClear()
{
#if ESP_ARDUINO_VERSION_MAJOR < 3
	ledcWrite(_audioChannel, 0);
#else
	ledcWrite(_pins.dacPin, 0);
#endif
}

// ------------------------------------------------------------------
//
// Button related methods
//
// ------------------------------------------------------------------
OneButton* JBWoprDevice::getButtonFrontLeft() {
	return _buttonFrontLeft;
}

OneButton* JBWoprDevice::getButtonFrontRight() {
	return _buttonFrontRight;
}

OneButton* JBWoprDevice::getButtonBackTop() {
	return _buttonBackTop;
}

OneButton* JBWoprDevice::getButtonBackBottom() {
	return _buttonBackBottom;
}

void JBWoprDevice::buttonFrontLeftSetClickCallback( std::function<void()> func ) {
	_buttonFrontLeftClickCallback = func;
}

void JBWoprDevice::buttonFrontLeftSetDoubleClickCallback( std::function<void()> func ) {
	_buttonFrontLeftDoubleClickCallback = func;
}

void JBWoprDevice::buttonFrontRightSetClickCallback( std::function<void()> func ) {
	_buttonFrontRightClickCallback = func;
}

void JBWoprDevice::buttonFrontRightSetDoubleClickCallback( std::function<void()> func ) {
	_buttonFrontRightDoubleClickCallback = func;
}

void JBWoprDevice::buttonBackTopSetClickCallback( std::function<void()> func ) {
	_buttonBackTopClickCallback = func;
}

void JBWoprDevice::buttonBackTopSetDoubleClickCallback( std::function<void()> func ) {
	_buttonBackTopDoubleClickCallback = func;
}

void JBWoprDevice::buttonBackBottomSetClickCallback( std::function<void()> func ) {
	_buttonBackBottomClickCallback = func;
}

void JBWoprDevice::buttonBackBottomSetDoubleClickCallback( std::function<void()> func ) {
	_buttonBackBottomDoubleClickCallback = func;
}

void JBWoprDevice::_staticButtonFrontLeftClickCallback(void* data) {
	JBWoprDevice* instance = static_cast<JBWoprDevice*>(data);
	instance->_buttonFrontLeftClick();
}

void JBWoprDevice::_staticButtonFrontLeftDoubleClickCallback(void* data) {
	JBWoprDevice* instance = static_cast<JBWoprDevice*>(data);
	instance->_buttonFrontLeftDoubleClick();
}

void JBWoprDevice::_staticButtonFrontRightClickCallback(void* data) {
	JBWoprDevice* instance = static_cast<JBWoprDevice*>(data);
	instance->_buttonFrontRightClick();
}

void JBWoprDevice::_staticButtonFrontRightDoubleClickCallback(void* data) {
	JBWoprDevice* instance = static_cast<JBWoprDevice*>(data);
	instance->_buttonFrontRightDoubleClick();
}

void JBWoprDevice::_staticButtonBackTopClickCallback(void* data) {
	JBWoprDevice* instance = static_cast<JBWoprDevice*>(data);
	instance->_buttonBackTopClick();
}

void JBWoprDevice::_staticButtonBackTopDoubleClickCallback(void* data) {
	JBWoprDevice* instance = static_cast<JBWoprDevice*>(data);
	instance->_buttonBackTopDoubleClick();
}

void JBWoprDevice::_staticButtonBackBottomClickCallback(void* data) {
	JBWoprDevice* instance = static_cast<JBWoprDevice*>(data);
	instance->_buttonBackBottomClick();
}

void JBWoprDevice::_staticButtonBackBottomDoubleClickCallback(void* data) {
	JBWoprDevice* instance = static_cast<JBWoprDevice*>(data);
	instance->_buttonBackBottomDoubleClick();
}

// ------------------------------------------------------------------
//
// Logger related methods
//
// ------------------------------------------------------------------
void JBWoprDevice::setLogLevel(LogLevel level) {
	_log->setLogLevel(level);
}

LogLevel JBWoprDevice::getLogLevel() {
	return _log->getLogLevel();
}

// ------------------------------------------------------------------
//
// Internal methods
//
// ------------------------------------------------------------------
//
// ------------------------------------------------------------------
//
// Button related methods
//
// ------------------------------------------------------------------
JBDefconLevel JBWoprDevice::_getDefconLevel(std::string value) {
	JBDefconLevel result = JBDefconLevel::DEFCON_NONE;
	if (value.find("DEFCON") == 0) {
		char numericChar = value[value.length() - 1];
		if (numericChar >= '1' && numericChar <= '5') {
			result = (JBDefconLevel)(numericChar - '1');
		}
	}
	return result;
}

uint32_t JBWoprDevice::_getDefconLedsPixel(JBDefconLevel level) {
	return 4 - level;
}

std::string JBWoprDevice::_getDefconLevelString(JBDefconLevel level) {
	switch (level) {
		case JBDefconLevel::DEFCON_NONE:
			return "DEFCON_NONE";
		default:
			return std::string("DEFCON_") + std::to_string(level + 1);
	}
}

// ------------------------------------------------------------------
//
// Button related methods
//
// ------------------------------------------------------------------
void JBWoprDevice::_buttonFrontLeftClick()
{
	_log->trace("Front left button click");
	if (_buttonFrontLeftClickCallback != nullptr) {
		_buttonFrontLeftClickCallback();
	}
}

void JBWoprDevice::_buttonFrontLeftDoubleClick()
{
	_log->trace("Front left button double click");
	if (_buttonFrontLeftDoubleClickCallback != nullptr) {
		_buttonFrontLeftDoubleClickCallback();
	}
}

void JBWoprDevice::_buttonFrontRightClick()
{
	_log->trace("Front right button click");
	if (_buttonFrontRightClickCallback != nullptr) {
		_buttonFrontRightClickCallback();
	}
}

void JBWoprDevice::_buttonFrontRightDoubleClick()
{
	_log->trace("Front right button double click");
	if (_buttonFrontRightDoubleClickCallback != nullptr) {
		_buttonFrontRightDoubleClickCallback();
	}
}

void JBWoprDevice::_buttonBackTopClick()
{
	_log->trace("Back top button click");
	if (_buttonBackTopClickCallback != nullptr) {
		_buttonBackTopClickCallback();
	}
}

void JBWoprDevice::_buttonBackTopDoubleClick()
{
	_log->trace("Back top button double click");
	if (_buttonBackTopDoubleClickCallback != nullptr) {
		_buttonBackTopDoubleClickCallback();
	}
}

void JBWoprDevice::_buttonBackBottomClick()
{
	_log->trace("Back bottom button click");
	if (_buttonBackBottomClickCallback != nullptr) {
		_buttonBackBottomClickCallback();
	}
}

void JBWoprDevice::_buttonBackBottomDoubleClick()
{
	_log->trace("Back bottom button double click");
	if (_buttonBackBottomDoubleClickCallback != nullptr) {
		_buttonBackBottomDoubleClickCallback();
	}
}

// ------------------------------------------------------------------
//
// Audio
//
// ------------------------------------------------------------------


