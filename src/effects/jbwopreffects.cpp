/// @file jbwopreffects.cpp
/// @author Jonny Bergdahl
/// @brief Source file for the JBWopr library file
/// @details Contains standard effect for the JBWopr library.
/// This code is distributed under the MIT License. See LICENSE for details.
/// @date Created: 2023-10-27
///
#include "jbwopreffects.h"
#include "jbwopr.h"
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <esp_random.h>
#include <numeric>
#include <string>
#include <utility>
#include <time.h>

JBWoprEffectBase::JBWoprEffectBase(JBWoprDevice *woprDevice, uint32_t duration, std::string name) {
	_woprDevice = woprDevice;
	_duration = duration;
	_name = name;
}

void JBWoprEffectBase::start() {
	_log.setLogLevel(_woprDevice->getLogLevel());
	_log.trace("Starting effect %s, duration=%i", getName().c_str(), _duration);
	_startTime = millis();
	_isRunning = true;
}

void JBWoprEffectBase::stop() {
	_log.trace("Stopping effect %s", getName().c_str());
	_woprDevice->displayClear();
	_woprDevice->defconLedsClear();
	_woprDevice->audioClear();
	_isRunning = false;
}

void JBWoprEffectBase::loop() {
	if (_duration == -1) {
		return;
	}
	if (!_done) {
		_startTime = millis();
	}
	else if (_startTime + _duration < millis()) {
		stop();
	}
}

std::string JBWoprEffectBase::getName() {
	return _name;
}

bool JBWoprEffectBase::isRunning() const {
	return _isRunning;
}

uint32_t JBWoprEffectBase::getDuration() const {
	return _duration;
}

void JBWoprEffectBase::_displayText(const std::string& text, JBTextAlignment alignment)
{
	uint8_t curDisplay = 0;
	uint8_t curDigit = 0;
	std::string displayText = text;
	size_t textLength = displayText.length();
	uint32_t padSize = (12 - textLength) / 2;
	uint32_t startIndex;
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

	auto display = _woprDevice->getDisplay();
	for ( uint8_t i = 0; i < 12; i++ )
	{
		if (i < startIndex || i >= endIndex) {
			display[curDisplay].writeDigitAscii(curDigit, ' ');
		} else {
			display[curDisplay].writeDigitAscii(curDigit, displayText.at(i - startIndex));
		}
		curDigit++;
		if (curDigit == 4)
		{
			curDigit = 0;
			curDisplay++;
		}
	}

	display[0].writeDisplay();
	display[1].writeDisplay();
	display[2].writeDisplay();
}

// ============================================
//
// DisplayTextDisplayEffect
//
JBWoprTextDisplayEffect::JBWoprTextDisplayEffect(JBWoprDevice *woprDevice,
												 const std::string& text,
												 JBTextAlignment alignment,
												 uint32_t duration,
												 const std::string& name)
	: JBWoprEffectBase(woprDevice, duration, name) {
	_alignment = alignment;
	setText(text);
}

void JBWoprTextDisplayEffect::start() {
	JBWoprEffectBase::start();
	_displayText(_text, _alignment);
}

void JBWoprTextDisplayEffect::loop() {
	if (!_isRunning) {
		return;
	}
	JBWoprEffectBase::loop();
	if (!_isRunning) {
		_woprDevice->displayClear();
	}
}

void JBWoprTextDisplayEffect::setText(const std::string& text) {
	switch (_alignment) {
		case JBTextAlignment::RIGHT:
			_text = JBStringHelper::getRightAlignedString(text, 12);
			break;
		case JBTextAlignment::CENTER:
			_text = JBStringHelper::getCenteredString(text, 12);
			break;
		default:
			_text = text;
			break;
	}
}

void JBWoprTextDisplayEffect::setAlignment(JBTextAlignment alignment) {
	_alignment = alignment;
}

// ============================================
//
// ScrollTextDisplayEffect
//
JBWoprScrollTextDisplayEffect::JBWoprScrollTextDisplayEffect(JBWoprDevice *woprDevice,
															 const std::string& text,
															 uint32_t scrollSpeed,
															 uint32_t duration,
															 const std::string& name) :
	JBWoprEffectBase(woprDevice, duration, name) {
	_text = text;
	_scrollSpeed = scrollSpeed;
}

void JBWoprScrollTextDisplayEffect::start() {
	JBWoprEffectBase::start();
	_log.setLogLevel(_woprDevice->getLogLevel());

	_done = false;
	_currentIndex = 0;
	_totalLength = _text.length() + 24;
	_endIndex = _totalLength - 12;
}

void JBWoprScrollTextDisplayEffect::loop() {
	if (!_isRunning) {
		return;
	}
	JBWoprEffectBase::loop();
	if (_nextTick > millis()) {
		return;
	}

	if (_currentIndex > _endIndex) {
		if (_duration == -1) {
			_startTime = millis();
			_currentIndex = 0;
		} else {
			_log.trace("Scrolling is done");
			_done = true;
			return;
		}
	}

	uint8_t curDisplay = 0;
	uint8_t curDigit = 0;
	size_t startIndex = 12;
	size_t endIndex = startIndex + _text.length();
	auto display = _woprDevice->getDisplay();

	curDisplay = 0;
	curDigit = 0;
	for (uint8_t j = 0; j < 12; j++ ) {
		if (_currentIndex + j < startIndex || _currentIndex + j >= endIndex) {
			display[curDisplay].writeDigitAscii(curDigit, ' ');
		} else {
			display[curDisplay].writeDigitAscii(curDigit, _text.at(_currentIndex - startIndex + j));
		}
		curDigit++;
		if (curDigit == 4) {
			curDigit = 0;
			curDisplay++;
		}
	}
	display[0].writeDisplay();
	display[1].writeDisplay();
	display[2].writeDisplay();

	_currentIndex++;
	_nextTick = millis() + _scrollSpeed;

	if (_currentIndex > _endIndex)
	{
		_startTime = millis();
		_nextTick = millis() + _duration;
	}
}

void JBWoprScrollTextDisplayEffect::setText(const std::string& text) {
	_text = text;
}

void JBWoprScrollTextDisplayEffect::setScrollSpeed(uint32_t scrollSpeed) {
	_scrollSpeed = scrollSpeed;
}


// ============================================
//
// TimeDisplayEffect
//
JBWoprTimeDisplayEffect::JBWoprTimeDisplayEffect(JBWoprDevice *woprDevice,
												 std::string  timeFormat,
												 uint32_t duration,
												 const std::string& name) :
	JBWoprEffectBase(woprDevice, duration, name),
	_rawTimeFormat(std::move(timeFormat)) {
}

void JBWoprTimeDisplayEffect::start() {
	_log.setLogLevel(_woprDevice->getLogLevel());
	setTimeFormat(_rawTimeFormat);
	JBWoprEffectBase::start();
}

void JBWoprTimeDisplayEffect::loop() {
	char timeChars[12];
	tm timeinfo {};

	if (!_isRunning) {
		return;
	}

	JBWoprEffectBase::loop();
	if (_nextLedTick > millis()) {
		return;
	}

	auto leds = _woprDevice->getDefconLeds();
	_pixelHue += 256;
	for (uint32_t i = 0; i < 5; i++) {
		uint16_t pixelHue = _pixelHue + (i * 65536L / 5);
		leds->setPixelColor(i, leds->gamma32(leds->ColorHSV(pixelHue)));
	}
	leds->show();
	_nextLedTick = millis() + 40;

	if (_nextTick > millis()) {
		return;
	}

	if (!JBTimeHelper::getTime(&timeinfo)) {
		_log.error("Failed to obtain time");
		_woprDevice->displayShowText("No time", JBTextAlignment::CENTER);
		_displayText("Time failed", JBTextAlignment::CENTER);
	} else {
		_evenFormat = !_evenFormat;
		auto timeFormat = _evenFormat ? _timeFormatEven : _timeFormatOdd;
		strftime(timeChars, 12, timeFormat.c_str(), &timeinfo);
		_displayText(timeChars, JBTextAlignment::CENTER);
	}

	_nextTick = millis() + 500;
}

void JBWoprTimeDisplayEffect::setTimeFormat(const std::string& timeFormat) {
	std::string format = timeFormat;
	if (format.empty()) {
		format = _woprDevice->getConfiguration()->timeFormat;
	}
	if (format.empty()) {
		format = "%H:%M:%S";
	}

	_timeFormatEven = format;
	_timeFormatOdd = _getOddTimeFormat(format);
}

std::string JBWoprTimeDisplayEffect::_getOddTimeFormat(const std::string &format) {
	std::string result = "";
	for (char ch: format) {
		if (ch == '%' || std::isalpha(ch)) {
			result += ch;
		} else {
			result += ".";
		}
	}
	return result;
}

// ============================================
//
// DateDisplayEffect
//
// Constructor
JBWoprDateDisplayEffect::JBWoprDateDisplayEffect(JBWoprDevice *woprDevice,
												 std::string dateFormat,
												 uint32_t duration,
												 const std::string& name) :
	JBWoprEffectBase(woprDevice, duration, name),
	_rawDateFormat(std::move(dateFormat)) {
}

void JBWoprDateDisplayEffect::start() {
	auto config = _woprDevice->getConfiguration();
	setDateFormat(_rawDateFormat);
	JBWoprEffectBase::start();
}

void JBWoprDateDisplayEffect::loop() {
	char text[12];
	tm timeinfo{};

	if (!_isRunning) {
		return;
	}

	JBWoprEffectBase::loop();
	if (_nextLedTick < millis()) {
		auto leds = _woprDevice->getDefconLeds();
		_pixelHue += 256;
		for (uint32_t i = 0; i < 5; i++) {
			uint16_t pixelHue = _pixelHue + (i * 65536L / 5);
			leds->setPixelColor(i, leds->gamma32(leds->ColorHSV(pixelHue)));
		}
		leds->show();
		_nextLedTick = millis() + 40;
	}

	if (_nextTick > millis()) {
		return;
	}

	if (!JBTimeHelper::getTime(&timeinfo)) {
		_log.error("Failed to obtain time");
		_displayText("Time failed");
	} else {
		strftime(text, 12, _dateFormat.c_str(), &timeinfo);
		_displayText(text, JBTextAlignment::CENTER);
	}

	_nextTick = millis() + 1000;
}

void JBWoprDateDisplayEffect::setDateFormat(const std::string& dateFormat) {
	std::string format = dateFormat;
	if (format.empty()) {
		format = _woprDevice->getConfiguration()->dateFormat;
	}
	if (format.empty()) {
		format = "%Y-%m-%d";
	}
	_dateFormat = format;
}

// ============================================
//
// DateTimeDisplayEffect
//
JBWoprDateTimeDisplayEffect::JBWoprDateTimeDisplayEffect(JBWoprDevice *woprDevice,
														 std::string  timeFormat,
														 std::string  dateFormat,
														 uint32_t duration,
														 const std::string& name) :
	JBWoprEffectBase(woprDevice, duration, name),
	_rawTimeFormat(std::move(timeFormat)),
	_rawDateFormat(std::move(dateFormat)) {
}

void JBWoprDateTimeDisplayEffect::start() {
	_log.setLogLevel(_woprDevice->getLogLevel());
	setTimeFormat(_rawTimeFormat);
	setDateFormat(_rawDateFormat);
	JBWoprEffectBase::start();
}

void JBWoprDateTimeDisplayEffect::loop() {
	char text[12];
	tm timeinfo{};

	if (!_isRunning) {
		return;
	}

	JBWoprEffectBase::loop();
	if (_nextLedTick > millis()) {
		return;
	}

	auto leds = _woprDevice->getDefconLeds();
	_pixelHue += 256;
	for (uint32_t i = 0; i < 5; i++) {
		uint16_t pixelHue = _pixelHue + (i * 65536L / 5);
		leds->setPixelColor(i, leds->gamma32(leds->ColorHSV(pixelHue)));
	}
	leds->show();
	_nextLedTick = millis() + 40;

	if (_nextTick > millis()) {
		return;
	}

	_displayCounter++;
	if (_displayCounter > 9) {
		_displayCounter = 0;
	}

	if (!JBTimeHelper::getTime(&timeinfo)) {
		_log.error("Failed to obtain time");
		_displayText("Time failed");
	} else {
		if (_displayCounter < 7) {
			_evenFormat = !_evenFormat;
			auto timeFormat = _evenFormat ? _timeFormatEven : _timeFormatOdd;
			strftime(text, 12, timeFormat.c_str(), &timeinfo);
			_displayText(text, JBTextAlignment::CENTER);

		} else {
			strftime(text, 12, _dateFormat.c_str(), &timeinfo);
			_displayText(text, JBTextAlignment::CENTER);
		}
	}
	_nextTick = millis() + 500;
}

void JBWoprDateTimeDisplayEffect::setTimeFormat(const std::string& timeFormat) {
	std::string format = timeFormat;
	if (format.empty()) {
		format = _woprDevice->getConfiguration()->timeFormat;
	}
	if (format.empty()) {
		format = "%H:%M:%S";
	}

	_timeFormatEven = format;
	_timeFormatOdd = _getOddTimeFormat(format);
}

void JBWoprDateTimeDisplayEffect::setDateFormat(const std::string& dateFormat) {

	std::string format = dateFormat;
	if (format.empty()) {
		format = _woprDevice->getConfiguration()->dateFormat;
	}
	if (format.empty()) {
		format = "%Y-%m-%d";
	}
	_dateFormat = format;
}

std::string JBWoprDateTimeDisplayEffect::_getOddTimeFormat(const std::string &format) {
	std::string result = "";
	for (char ch: format) {
		if (ch == '%' || std::isalpha(ch)) {
			result += ch;
		} else {
			result += ".";
		}
	}
	return result;
}

// ============================================
//
// XmasSecondsDisplayEffect
//
JBWoprXmasSecondsDisplayEffect::JBWoprXmasSecondsDisplayEffect(JBWoprDevice *woprDevice,
															   uint32_t scrollSpeed,
															   uint32_t duration,
															   const std::string& name) :
	JBWoprScrollTextDisplayEffect(woprDevice, "", scrollSpeed, duration, name) {
}

void JBWoprXmasSecondsDisplayEffect::start() {
	uint32_t secondsUntilXmas = 0;
	struct tm localTime{};
	if (!getLocalTime(&localTime)) {
		_log.error("Failed to obtain time");
		_text = "Failed to obtain time";
		return;
	}
	time_t now = mktime(&localTime);
	int32_t nextXmasYear = localTime.tm_mon == 11 && localTime.tm_mday > 24 ? localTime.tm_year + 1 : localTime.tm_year;
	tm christmas = {0, 0, 0, 25, 11, nextXmasYear};
	time_t nextChristmas = mktime(&christmas);
	secondsUntilXmas = nextChristmas - now;
	JBWoprScrollTextDisplayEffect::setText(std::to_string(secondsUntilXmas) + std::string(" Seconds until XMAS..."));
	JBWoprScrollTextDisplayEffect::start();
}

void JBWoprXmasSecondsDisplayEffect::setText(std::string &text) {
	// Nothing to do
}

// ============================================
//
// WoprMovieDisplayEffect
//
JBWoprMissileCodeSolveEffect::JBWoprMissileCodeSolveEffect(JBWoprDevice *woprBoard,
														   CodeSolveVariant solveVariant,
														   uint32_t duration,
														   const std::string& name)
   : JBWoprEffectBase(woprBoard, duration, name),
	 _solveVariant(solveVariant) {
}

void JBWoprMissileCodeSolveEffect::start() {
	_currentSolveStep = 0;
	_nextSolveTick = millis() + _getNextSolveTicks();
	_currentSolution = _getSolution();
	_currentGuess = _getStartingGuess();
	_codeSolveOrder = _getSolveOrder();
	JBWoprEffectBase::start();
}

void JBWoprMissileCodeSolveEffect::loop() {
	if (_nextTick > millis()) {
		return;
	}
	_nextTick = millis() + 100;
	if (_currentSolveStep < _codeSolveOrder.size()) {
		_displayCurrentGuess();
	}

	if (_nextSolveTick < millis()) {
		// Do next step
		if (_currentSolveStep < _codeSolveOrder.size()) {
			// Still solving
			uint32_t index = _codeSolveOrder[_currentSolveStep];
			_currentGuess[index] = _currentSolution[index];
			_nextSolveTick = millis() + _getNextSolveTicks();
			_displaySolvedCharacters();
			_nextTick = millis() + 500;
			_currentSolveStep++;
		}
	}
	if (_currentSolveStep >= _codeSolveOrder.size()) {
		if (_currentSolveStep < _codeSolveOrder.size() + 6) {
			_displayBlinkingSolution();
			_nextTick = millis() + 800;
		} else if (_currentSolveStep < _codeSolveOrder.size() + 12) {
			_displayBlinkingLaunching();
			_nextTick = millis() + 800;
		} else {
			// Done
			_woprDevice->audioClear();
			JBWoprEffectBase::stop();
		}
		_currentSolveStep++;
	}
}

void JBWoprMissileCodeSolveEffect::setCodeSolveVariant(CodeSolveVariant solveVariant) {
	_solveVariant = solveVariant;
}

void JBWoprMissileCodeSolveEffect::_displayCurrentGuess() {
	// We are still in the process of solving
	std::string text = "";
	for (uint32_t i = 0; i < 12; i++) {
		if (_currentGuess[i] == '*') {
			text += ' ';
		} else if (_currentGuess[i] == _currentSolution[i]) {
			text += _currentSolution[i];
		} else {
			text += _getRandomChar();
		}
	}
	_woprDevice->audioPlayTone(random(90, 250));
	int32_t percentage = 100 * _currentSolveStep / _codeSolveOrder.size();
	int32_t defconValue = map(percentage, 0, 100, 4, 0);
	if (defconValue != _defconValue) {
		_defconValue = defconValue;
		JBDefconLevel defconLevel = (JBDefconLevel)defconValue;
		_woprDevice->defconLedsSetDefconLevel(defconLevel);
	}
	_woprDevice->displayShowText(text);
}

void JBWoprMissileCodeSolveEffect::_displaySolvedCharacters() {
	// We are still in the process of solving
	std::string text = "";
	for (uint32_t i = 0; i < 12; i++) {
		if (_currentGuess[i] == '*') {
			text += ' ';
		} else if (_currentGuess[i] == _currentSolution[i]) {
			text += _currentSolution[i];
		} else {
			text += '*';
		}
	}
	_woprDevice->audioPlayNote(NOTE_G, 5);
	_woprDevice->displayShowText(text);
}

void JBWoprMissileCodeSolveEffect::_displayBlinkingSolution() {
	if (_currentSolveStep % 2) {
		_woprDevice->audioClear();
		_woprDevice->displayClear();
		_woprDevice->defconLedsSetColor(0x000000);
	} else {
		_woprDevice->audioPlayNote(NOTE_G, 5);
		_woprDevice->displayShowText(_currentSolution);
		_woprDevice->displayShow();
		_woprDevice->defconLedsSetDefconLevel(JBDefconLevel::DEFCON_1);
	}
}

void JBWoprMissileCodeSolveEffect::_displayBlinkingLaunching() {
	if (_currentSolveStep % 2) {
		_woprDevice->audioClear();
		_woprDevice->displayClear();
		_woprDevice->defconLedsSetColor(0x000000);
	} else {
		_woprDevice->audioPlayNote(NOTE_G, 5);
		_woprDevice->displayShowText("LAUNCHING...");
		_woprDevice->displayShow();
		_woprDevice->defconLedsSetColor(0xFF0000);
	}
}

std::string JBWoprMissileCodeSolveEffect::_getRandomCode() {
	std::string result;
	for (uint32_t i = 0; i < 12; ++i) {
		result += _getRandomChar();
	}
	return result;
}

char JBWoprMissileCodeSolveEffect::_getRandomChar() {
	uint32_t rand = random(0, 38);
	if (rand < 10) {
		return '0' + rand;
	} else if (rand < 36) {
		return 'A' + rand - 10;
	} else {
		return ' ';
	}
}

uint32_t JBWoprMissileCodeSolveEffect::_getNextSolveTicks() const {
	return random(_minSolveTicks, _maxSolveTicks);
}

std::string JBWoprMissileCodeSolveEffect::_getSolution() {
	switch (_solveVariant) {
		case CodeSolveVariant::MOVIE:
			return "CPE 1704 TKS";
			break;
		case CodeSolveVariant::MESSAGE:
			return "LOLZ FOR YOU";
			break;
		default:
			return _getRandomCode();
			break;
	}
}

std::string JBWoprMissileCodeSolveEffect::_getStartingGuess() {
	switch (_solveVariant) {
		case CodeSolveVariant::MOVIE:
			return "---*----*---";
			break;
		default:
			return "------------";
			break;
	}
}

std::vector<uint32_t> JBWoprMissileCodeSolveEffect::_getSolveOrder() {
	switch (_solveVariant) {
		case CodeSolveVariant::MOVIE:
			return std::vector<uint32_t>{7, 1, 4, 6, 11, 2, 5, 0, 10, 9};
		default:
			std::vector<uint32_t> result(12);
			std::iota(std::begin(result), std::end(result), 0);
			for (int i = 0; i < result.size() - 1; i++) {
				int j = i + esp_random() % (result.size() - i);
				std::swap(result[i], result[j]);
			}
			return result;
	}
}

// ============================================
//
// JBWoprDefconRainbowEffect
//
JBWoprDefconRainbowEffect::JBWoprDefconRainbowEffect(JBWoprDevice *woprDevice,
													 uint32_t duration,
													 const std::string& name)
	: JBWoprEffectBase(woprDevice, duration, name) {
}

void JBWoprDefconRainbowEffect::loop() {
	JBWoprEffectBase::loop();

	if (!_isRunning || _nextTick > millis()) {
		return;
	}

	auto leds = _woprDevice->getDefconLeds();
	_pixelHue += 256;
	for (uint32_t i = 0; i < 5; i++) {
		uint16_t pixelHue = _pixelHue + (i * 65536L / 5);
		leds->setPixelColor(i, leds->gamma32(leds->ColorHSV(pixelHue)));
	}
	leds->show();
	_nextTick = millis() + 40;
}

// ============================================
//
// JBWoprSongEffectBase
//
JBWoprSongEffect::JBWoprSongEffect(JBWoprDevice *woprBoard,
								   const std::vector<Note>* song,
								   uint32_t tempo,
								   uint32_t duration,
								   const std::string& name)
	: JBWoprEffectBase(woprBoard, duration, name),
	_song(song),
	_tempo(tempo),
	_wholeNote((60000 * 4) / tempo) {
}

void JBWoprSongEffect::start() {
	_done = false;
	_step = 0;
	JBWoprEffectBase::start();
}

void JBWoprSongEffect::loop() {
	if (_done) {
		JBWoprEffectBase::loop();
		return;
	}
	if (_nextTick > millis()) {
		return;
	}

	if (_step >= _song->size()) {
		_woprDevice->audioClear();
		_done = true;
		_isRunning = false;
		return;
	}

	// calculates the duration of each note
	Note note = _song->at(_step);
	int32_t divider = note.duration;
	uint32_t noteDuration = 0;
	if (divider == 0)
	{
		noteDuration = 0;
	} else if (divider > 0) {
		// regular note, just proceed
		noteDuration = (_wholeNote) / divider;
	} else if (divider < 0) {
		// dotted notes are represented with negative durations!!
		noteDuration = (_wholeNote) / abs(divider);
		noteDuration *= 1.5; // increases the duration in half for dotted notes
	}

	_woprDevice->audioPlayNote((note_t)note.note, note.octave);

	std::string text = note.text;

	if (!text.empty())
	{
		if (text != "-")
		{
			_woprDevice->displayShowText(text, JBTextAlignment::CENTER);
		}
		else
		{
			_woprDevice->displayClear();
		}
	}

	_step++;
	_nextTick = millis() + noteDuration;
}

void JBWoprSongEffect::setSong(const std::vector<Note>* song) {
	_song = song;
}

void JBWoprSongEffect::setTempo(uint32_t tempo) {
	_tempo = tempo;
	_wholeNote = (60000 * 4) / tempo;
}

