/// @file jbwopreffects.h
/// @author Jonny Bergdahl
/// @brief  Header file for the JBWopr library.
/// @details Contains declarations for standard effects for the JBWopr library.
/// @date Created: 2023-10-27
/// @copyright Copyright© 2023, Jonny Bergdahl
///
#ifndef ARDUINO_WOPR_JBWOPREFFECTS_H
#define ARDUINO_WOPR_JBWOPREFFECTS_H

#include <Arduino.h>
#include <JBLogger.h>
#include "jbwoprhelpers.h"
#include <string>
#include <vector>

class JBWoprDevice;

// Note that effect names needs to fit in 12 characters.
#define JBWOPR_EFFECT_NAME_BASE 			"Base"             	///< Name of JBWoprEffectBase
#define JBWOPR_EFFECT_NAME_TEXT 			"Text"             	///< Name of JBWoprDisplayTextEffect
#define JBWOPR_EFFECT_NAME_SCROLLTEXT 		"Scroll text"		///< Name of JBWoprDisplayScrollTextEffect
#define JBWOPR_EFFECT_NAME_TIME 			"Time"             	///< Name of JBWoprDisplayTimeEffect
#define JBWOPR_EFFECT_NAME_DATE 			"Date"             	///< Name of JBWoprDisplayDateEffect
#define JBWOPR_EFFECT_NAME_DATETIME 		"Date Time"			///< Name of JBWoprDisplayDateTimeEffect
#define JBWOPR_EFFECT_NAME_XMAS_SECONDS		"Xmas seconds"     	///< Name of JBWoprDisplayXmasSecondsEffect
#define JBWOPR_EFFECT_NAME_CODE_SOLVE 		"Code Solve" 		///< Name of JBWoprWOPRMovieSolveEffect
#define JBWOPR_EFFECT_NAME_DEFCON_RAINBOW 	"Rainbow"			///< Name of JBWoprDefconRainbowEffect
#define JBWOPR_EFFECT_NAME_SONG				"Song"				///< Name of JBWoprSongEffect

/// @brief Code solve variant for the JBWoprMissileCodeSolveEffect class
enum CodeSolveVariant {
	MOVIE, 				///< Movie code solve
	RANDOM,				///< Random code solve
	MESSAGE				///< Message code solve
};

/// @brief Abstract effect base class
class JBWoprEffectBase {
public:
	/// @brief Constructor
	/// @param WOPRDevice instance
	/// @param duration (optional) Duration of effect in milliseconds, default is -1 (infinite)
	/// @param name (optional) Name of effect
	explicit JBWoprEffectBase(JBWoprDevice *woprDevice,
							  uint32_t duration = -1,
							  std::string name = JBWOPR_EFFECT_NAME_BASE);

	/// @brief Get name of effect
	/// @return Name of effect
	virtual std::string getName();

	/// @brief Start effect
	virtual void start();

	/// @brief Stop effect
	virtual void stop();

	/// @brief Run loop
	virtual void loop();
	/// @brief Check if effect is running
	/// @return True if effect is running
	bool isRunning() const;

	/// @brief Set duration of effect
	/// @param duration Duration of effect in milliseconds
	uint32_t getDuration() const;

protected:
	JBWoprDevice *_woprDevice;			///< JBWoprDevice instance
	std::string _name;					///< Name of effect
	bool _isRunning = false;			///< True if effect is running
	bool _done = true;					///< True if effect is done, waiting for duration to end
	uint32_t _duration = -1;			///< Duration of effect in milliseconds
	uint32_t _startTime = 0;			///< Start time of effect in milliseconds
	uint32_t _nextTick = 0; 			///< Next tick time in milliseconds

	/// @brief Display text on raw display
	/// @details This method will display text on the raw display without using
	/// the displayShowText() method of the JBWoprDevice class.
	/// @param text Text to display
	/// @param alignment (optional) Text alignment, default is LEFT
	void _displayText(const std::string& text, JBTextAlignment alignment = JBTextAlignment::LEFT);

private:
	JBLogger _log {"effect" };	///< Logger instance
};

/// @brief Display effect for showing text
class JBWoprTextDisplayEffect : public JBWoprEffectBase {
public:
	/// @brief Constructor
	/// @param woprDevice JBWoprDevice instance
	/// @param text Text to display
	/// @param alignment (optional) Text alignment, default is LEFT
	/// @param duration (optional) Duration of effect in milliseconds, default is -1 (infinite)
	/// @param name (optional) Name of effect
	explicit JBWoprTextDisplayEffect(JBWoprDevice *woprDevice,
									 const std::string& text,
									 JBTextAlignment alignment = JBTextAlignment::LEFT,
									 uint32_t duration = -1,
									 const std::string& name=JBWOPR_EFFECT_NAME_TEXT);

	/// @brief Start effect
	void start() override;

	/// @brief Run loop
	void loop() override;

	/// @brief Set text to display
	/// @param text Text to display
	virtual void setText(const std::string& text);

	/// @brief Set center text
	/// @param value True if text should be centered
	virtual void setAlignment(JBTextAlignment alignment);

protected:
	std::string _text;				///< Text to display
	JBTextAlignment _alignment;        ///< True if text should be centered
};

/// @brief Display effect for scrolling text
class JBWoprScrollTextDisplayEffect : public JBWoprEffectBase {
public:
	/// @brief Constructor
	/// @param woprDevice JBWoprDevice instance
	/// @param text Text to display
	/// @param scrollSpeed (optional) Scroll speed in milliseconds, default is 200
	/// @param duration (optional) Duration after scrolling in milliseconds, default is -1 (infinite)
	/// @param name (optional) Name of effect
	explicit JBWoprScrollTextDisplayEffect(JBWoprDevice *woprDevice,
										   const std::string& text,
										   uint32_t scrollSpeed = 200,
										   uint32_t duration = -1,
										   const std::string& name=JBWOPR_EFFECT_NAME_SCROLLTEXT);

	/// @brief Start effect
	void start() override;

	/// @brief Run loop
	void loop() override;

	/// @brief Set text to display
	/// @param text Text to display
	virtual void setText(const std::string& text);

	/// @brief Set scroll speed
	/// @param scrollSpeed Scroll speed in milliseconds
	void setScrollSpeed(uint32_t scrollSpeed);

protected:
	std::string _text;					///< Text to display
	uint32_t _scrollSpeed = 200;		///< Scroll speed in milliseconds
	size_t _currentIndex = 0;			///< Current character index
	size_t _totalLength = 0;			///< Total length of text
	size_t _endIndex = 0;				///< End index of text

private:
	JBLogger _log {"scroll" };	///< Logger instance
};

/// @brief Display effect for showing the current time
class JBWoprTimeDisplayEffect : public JBWoprEffectBase {
public:
	/// @brief Constructor
	/// @param woprDevice JBWoprDevice instance
	/// @param timeFormat (optional) Time format, default is fetched from config
	/// @param duration (optional) Duration of effect in milliseconds, default is -1 (infinite)
	/// @param name (optional) Name of effect
	explicit JBWoprTimeDisplayEffect(JBWoprDevice *woprDevice,
									 std::string  timeFormat = "",
									 uint32_t duration = -1,
									 const std::string& name=JBWOPR_EFFECT_NAME_TIME);

	/// @brief Start effect
	void start() override;

	/// @brief Run loop
	void loop() override;

	/// @brief Set time format
	/// @param timeFormat Time format
	virtual void setTimeFormat(const std::string& timeFormat);

protected:
	/// @brief Get time format
	/// @param format
	/// @return Time format
	virtual std::string _getOddTimeFormat(const std::string& format);

	bool _evenFormat = false;			///< True if even format
	std::string _rawTimeFormat;			///< Raw time format
	std::string _timeFormatEven;		///< Time format for even
	std::string _timeFormatOdd;			///< Time format for odd
	uint64_t _nextLedTick = 0;			///< Next LED tick
	uint16_t _pixelHue = 0;				///< Pixel hue

private:
	JBLogger _log {"time" };			///< Logger instance
};

/// @brief Display effect for showing the current date
class JBWoprDateDisplayEffect : public JBWoprEffectBase {
public:
	explicit JBWoprDateDisplayEffect(JBWoprDevice *woprDevice,
									 std::string  dateFormat = "",
									 uint32_t duration = -1,
									 const std::string& name=JBWOPR_EFFECT_NAME_DATE);

	/// @brief Start effect
	void start() override;

	/// @brief Get name of effect
	void loop() override;

	/// @brief Set date format
	/// @param dateFormat Date format
	void setDateFormat(const std::string& dateFormat);

protected:
	std::string _rawTimeFormat;			///< Raw time format
	std::string _timeFormatEven;		///< Time format for even
	std::string _timeFormatOdd;			///< Time format for odd
	std::string _rawDateFormat;			///< Raw date format
	std::string _dateFormat;			///< Date format
	uint64_t _nextLedTick = 0;			///< Next LED tick
	uint16_t _pixelHue = 0;				///< Pixel hue

private:
	JBLogger _log {"date" };	///< Logger instance
};

/// @brief Display effect for showing the current date and time
/// @details Shows date for 2 seconds, then time for 8 seconds
class JBWoprDateTimeDisplayEffect : public JBWoprEffectBase {
public:
	/// @brief Constructor
	/// @param woprDevice JBWoprDevice instance
	explicit JBWoprDateTimeDisplayEffect(JBWoprDevice *woprDevice,
										 std::string  timeFormat = "",
										 std::string  dateFormat = "",
										 uint32_t duration = -1,
										 const std::string& name=JBWOPR_EFFECT_NAME_DATETIME);

	/// @brief Start effect
	void start() override;

	/// @brief Get name of effect
	void loop() override;

	/// @brief Set time format
	/// @param timeFormat Time format
	virtual void setTimeFormat(const std::string& timeFormat);

	/// @brief Set date format
	/// @param dateFormat Date format
	void setDateFormat(const std::string& dateFormat);

private:
	uint32_t _displayCounter = 0;		///< Display counter, for switching between Date and Time
	std::string _rawDateFormat;			///< Raw date format
	std::string _rawTimeFormat;			///< Raw time format
	std::string _dateFormat; 			///< Date format
	bool _evenFormat = false;			///< True if even format
	std::string _timeFormatEven;		///< Time format for even
	std::string _timeFormatOdd;			///< Time format for odd
	uint64_t _nextLedTick = 0;			///< Next LED tick
	uint16_t _pixelHue = 0;				///< Pixel hue
	JBLogger _log {"datetime" };		///< Logger instance

	/// @brief Get time format
	/// @param format
	/// @return Time format
	virtual std::string _getOddTimeFormat(const std::string& format);
};

/// @brief Display effect for showing seconds un til Xmas
class JBWoprXmasSecondsDisplayEffect: public JBWoprScrollTextDisplayEffect {
public:
	/// @brief Constructor
	/// @param woprBoard JBWoprDevice instance
	explicit JBWoprXmasSecondsDisplayEffect(JBWoprDevice *woprBoard,
											uint32_t scrollSpeed = 200,
											uint32_t duration = -1,
											const std::string& name=JBWOPR_EFFECT_NAME_XMAS_SECONDS);

	/// @brief Start effect
	/// @param duration Duration of effect (after it is done) in milliseconds
	void start() override;

protected:
	/// @brief Hide the method, as this effect uses it's own text
	void setText(std::string& text);

private:
	JBLogger _log {"xmas" };	///< Logger instance
};

/// @brief Display effect for showing the WOPR movie code solve
class JBWoprMissileCodeSolveEffect : public JBWoprEffectBase {
public:
	/// @brief Constructor
	/// @param woprBoard JBWoprDevice instance
	/// @param solveVariant Code solve variant, default is MOVIE
	/// @param duration Duration of effect (after it is done) in milliseconds, default is -1 (infinite)
	/// @param name Name of effect, default is JBWOPR_EFFECT_NAME_WOPR_MOVIE
	explicit JBWoprMissileCodeSolveEffect(JBWoprDevice *woprBoard,
										  CodeSolveVariant solveVariant = CodeSolveVariant::MOVIE,
										  uint32_t duration = -1,
										  const std::string& name=JBWOPR_EFFECT_NAME_CODE_SOLVE);

	/// @brief Start effect
	/// @param duration Duration of effect (after it is done) in milliseconds
	void start() override;

	/// @brief Run loop
	void loop() override;

	/// @brief Set code solve variant
	/// @param solveVariant Code solve variant
	void setCodeSolveVariant(CodeSolveVariant solveVariant);

private:
	/// @brief Display current solution
	void _displayCurrentGuess();

	/// @brief Display solved characters
	void _displaySolvedCharacters();

	/// @brief Display blinking solution
	void _displayBlinkingSolution();

	/// @brief Display blinking launching
	void _displayBlinkingLaunching();

	/// @brief Setup current solution
	/// @return Current solution
	std::string _getSolution();

	/// @brief Get starting guess
	/// @return Starting guess
	std::string _getStartingGuess();

	///	@brief  solve order
	std::vector<uint32_t> _getSolveOrder();

	/// @brief Get random code
	/// @return Random code
	std::string _getRandomCode();

	/// @brief Get a random char
	/// @return Random char
	static char _getRandomChar();

	/// @brief Get next solve ticks
	uint32_t _getNextSolveTicks() const;

	CodeSolveVariant _solveVariant = CodeSolveVariant::MOVIE; ///< Code solve variant
	uint32_t _currentSolveStep = 0;                          ///< Current solve step (0 - 9)
	uint32_t _nextSolveTick = 0;                             ///< Next tick for solve step
	int32_t _defconValue = -1;								 ///< Defcon level
	std::string _currentSolution;                            ///< Current solution
	std::string _currentGuess;                               ///< Current guess
	std::vector<uint32_t> _codeSolveOrder;                   ///< Code solve order
	const uint32_t _minSolveTicks = 1300;                    ///< Minimum solve ticks
	const uint32_t _maxSolveTicks = 2500;                    ///< Maximum solve ticks
};

/// @brief Rainbow effect for the DEFCON LED's
class JBWoprDefconRainbowEffect : public JBWoprEffectBase {
public:
	/// @brief Constructor
	/// @param woprBoard JBWoprDevice instance
	/// @param duration Duration of effect (after it is done) in milliseconds, default is -1 (infinite)
	/// @param name Name of effect, default is JBWOPR_EFFECT_NAME_DEFCON_RAINBOW
	explicit JBWoprDefconRainbowEffect(JBWoprDevice *woprBoard,
									   uint32_t duration = -1,
									   const std::string& name=JBWOPR_EFFECT_NAME_DEFCON_RAINBOW);

	/// @brief Run loop
	void loop() override;

private:
	uint16_t _pixelHue = 0;		///< Pixel hue
};

/// @brief Base class for song effects
class JBWoprSongEffect: public JBWoprEffectBase {
public:
	/// @brief Note struct
	struct Note {
		uint16_t note;			///< Note
		uint16_t octave;		///< Octave
		int16_t duration;		///< Duration, negative for dotted note
		std::string text;		///< Lyrics, set to "" to keep last one, or set to "-" to clear display
	};

	/// @brief Constructor
	/// @param woprBoard JBWoprDevice instance
	/// @param melody Melody
	/// @param tempo Tempo
	/// @param duration Duration of effect (after it is done) in milliseconds, default is -1 (infinite)
	/// @param name Name of effect, default is JBWOPR_EFFECT_NAME_SONG
	explicit JBWoprSongEffect(JBWoprDevice *woprBoard,
							  const std::vector<Note>* song,
							  uint32_t tempo = 114,
							  uint32_t duration = -1,
							  const std::string& name=JBWOPR_EFFECT_NAME_SONG);

	/// @brief Start effect
	void start() override;

	/// @brief Run loop
	void loop() override;

	/// @brief Set melody
	/// @param melody Melody
	virtual void setSong(const std::vector<Note>* song);

	/// @brief Set tempo
	/// @param tempo Tempo
	virtual void setTempo(uint32_t tempo);

protected:
	const std::vector<Note>*_song = nullptr;		///< Melody
	uint32_t _step = 0;								///< Current step
	uint32_t _tempo = 114;							///< Tempo
	uint32_t _wholeNote = (60000 * 4) / _tempo;		///< Whole note duration
	bool _done = false;								///< True if done

private:
	JBLogger _log {"song" };	///< Logger instance
};




#endif//ARDUINO_WOPR_JBWOPREFFECTS_H
