/// @file jbwoprtherickeffect.cpp
/// @author Jonny Bergdahl
/// @brief Main header file for the JBWopr library.
/// @details Contains declarations for the JBWoprTheRickEffect class.
/// This code is distributed under the MIT License. See LICENSE for details.
/// @date Created: 2023-11-15
/// @copyright Copyright© 2023, Jonny Bergdahl
//
#ifndef ARDUINO_WOPR_JBWOPRTHERICKEFFECT_H
#define ARDUINO_WOPR_JBWOPRTHERICKEFFECT_H

#include "jbwopreffects.h"

#define JBWOPR_EFFECT_NAME_THE_RICK "The Rick"		    ///< Name of JBWoprTheRickEffect

/// @brief The Rick effect
/// @ingroup EffectGroup
/// @details Original code: https://github.com/robsoncouto/arduino-songs
class JBWoprTheRickEffect : public JBWoprSongEffect {
public:
	/// @brief Constructor
	/// @ingroup EffectGroup
	/// @param woprBoard JBWoprDevice instance
	/// @param duration Duration of effect (after it is done) in milliseconds, default is -1 (infinite)
	/// @param name Name of effect, default is JBWOPR_EFFECT_NAME_THE_RICK
	explicit JBWoprTheRickEffect(JBWoprDevice *woprBoard,
								 uint32_t tempo = 114,
								 uint32_t duration = -1,
								 const std::string name=JBWOPR_EFFECT_NAME_THE_RICK);

protected:
	// Hide these as they are hard coded
	void setSong(const std::vector<JBWoprSongEffect::Note>* song) override {} ;
	void setTempo(uint32_t tempo) override {};

	/// @brief The Rick tune
	const std::vector<JBWoprSongEffect::Note> _theRick {
			{0,       0, 4,  ""},
			{NOTE_B,  4, 8,  "WE'RE"},
			{NOTE_Cs, 5, 8,  "NO"},
			{NOTE_D,  5, 8,  "STRANGERS"},
			{NOTE_D,  5, 8,  ""},
			{NOTE_E,  5, 8,  "TO"},
			{NOTE_Cs, 5, -8, "LOVE"},
			{NOTE_B,  4, 16, ""},
			{NOTE_A,  4, 2,  ""},
			{0,       0, 4,  "-"},
			{0,       0, 8,  ""},
			{NOTE_B,  4, 8,  "YOU"},
			{NOTE_B,  4, 8,  "KNOW"},
			{NOTE_Cs, 5, 8,  "THE"},
			{NOTE_D,  5, 8,  "RULES"},
			{NOTE_B,  4, 4,  ""},
			{NOTE_A,  4, 8,  "AND"},
			{NOTE_A,  5, 8,  "SO"},
			{0,       0, 8,  "DO"},
			{NOTE_A,  5, 8,  "I"},
			{NOTE_E,  5, -4, ""},
			{0,       0, 4,  ""},
			{NOTE_B,  4, 8,  "A"},
			{NOTE_B,  4, 8,  "FULL"},
			{NOTE_Cs, 5, 8,  "COMMITMENT'S"},
			{NOTE_D,  5, 8,  ""},
			{NOTE_B,  4, 8,  ""},
			{NOTE_D,  5, 8,  "WHAT"},
			{NOTE_E,  5, 8,  "I'M"},
			{0,       0, 8,  ""},
			{0,       0, 8,  ""},
			{NOTE_Cs, 5, 8,  "THINKING"},
			{NOTE_B,  4, 8,  ""},
			{NOTE_A,  4, -4, "OF"},
			{0,       0, 4,  "-"},
			{0,       0, 8,  ""},
			{NOTE_B,  4, 8,  "YOU"},
			{NOTE_B,  4, 8,  "WOULDN'T"},
			{NOTE_Cs, 5, 8,  ""},
			{NOTE_D,  5, 8,  "GET"},
			{NOTE_B,  4, 8,  "THIS"},
			{NOTE_A,  4, 4,  "FROM"},
			{NOTE_E,  5, 8,  "ANY"},
			{NOTE_E,  5, 8,  "OTHER"},
			{NOTE_E,  5, 8,  ""},
			{NOTE_Fs, 5, 8,  "GUY"},
			{NOTE_E,  5, 4,  ""},
			{0,       0, 4,  "-"},
			{NOTE_D,  5, 2,  "I"},
			{NOTE_E,  5, 8,  "JUST"},
			{NOTE_Fs, 5, 8,  "WANNA"},
			{NOTE_D,  5, 8,  "TELL"},
			{NOTE_E,  5, 8,  "YOU"},
			{NOTE_E,  5, 8,  "HOW"},
			{NOTE_E,  5, 8,  "I'M"},
			{NOTE_Fs, 5, 8,  ""},
			{NOTE_E,  5, 4,  "FEELING"},
			{NOTE_A,  4, 4,  ""},
			{0,       0, 2,  "-"},
			{NOTE_B,  4, 8,  "GOTTA"},
			{NOTE_Cs, 5, 8,  ""},
			{NOTE_D,  5, 8,  "MAKE"},
			{NOTE_B,  4, 8,  "YOU"},
			{0,       0, 8,  ""},
			{NOTE_E,  5, 8,  "UNDERSTAND"},
			{NOTE_Fs, 5, 8,  ""},
			{NOTE_E,  5, -4, ""},
			{NOTE_A,  4, 16, "NEVER"},
			{NOTE_B,  4, 16, ""},
			{NOTE_D,  5, 16, "GONNA"},
			{NOTE_B,  4, 16, ""},
			{NOTE_Fs, 5, -8, "GIVE"},
			{NOTE_Fs, 5, -8, "YOU"},
			{NOTE_E,  5, -4, "UP"},
			{NOTE_A,  4, 16, "NEVER"},
			{NOTE_B,  4, 16, ""},
			{NOTE_D,  5, 16, "GONNA"},
			{NOTE_B,  4, 16, ""},
			{NOTE_E,  5, -8, "LET"},
			{NOTE_E,  5, -8, "YOU"},
			{NOTE_D,  5, -8, "DOWN"},
			{NOTE_Cs, 5, 16, ""},
			{NOTE_B,  4, -8, "-"},
			{NOTE_A,  4, 16, "NEVER"},
			{NOTE_B,  4, 16, ""},
			{NOTE_D,  5, 16, "GONNA"},
			{NOTE_B,  4, 16, ""},
			{NOTE_D,  5, 4,  "RUN"},
			{NOTE_E,  5, 8,  "AROUND"},
			{NOTE_Cs, 5, -8, ""},
			{NOTE_B,  4, 16, "AND"},
			{NOTE_A,  4, 8,  ""},
			{NOTE_A,  4, 8,  "DESERT"},
			{NOTE_E,  5, 4,  ""},
			{NOTE_D,  5, 2,  "YOU"},
			{NOTE_A,  4, 16, "NEVER"},
			{NOTE_B,  4, 16, ""},
			{NOTE_D,  5, 16, "GONNA"},
			{NOTE_B,  4, 16, ""},
			{NOTE_Fs, 5, -8, "MAKE"},
			{NOTE_Fs, 5, -8, "YOU"},
			{NOTE_E,  5, -4, "CRY"},
			{NOTE_A,  4, 16, "NEVER"},
			{NOTE_B,  4, 16, ""},
			{NOTE_D,  5, 16, "GONNA"},
			{NOTE_B,  4, 16, ""},
			{NOTE_A,  5, 4,  "SAY"},
			{NOTE_Cs, 5, 8,  "GOODBYE"},
			{NOTE_D,  5, -8, ""},
			{NOTE_Cs, 5, 16, ""},
			{NOTE_B,  4, 8,  "-"},
			{NOTE_A,  4, 16, "NEVER"},
			{NOTE_B,  4, 16, "-"},
			{NOTE_D,  5, 16, "GONNA"},
			{NOTE_B,  4, 16, ""},
			{NOTE_D,  5, 4,  "TELL"},
			{NOTE_E,  5, 8,  "A"},
			{NOTE_Cs, 5, -8, "LIE"},
			{NOTE_B,  4, 16, ""},
			{NOTE_A,  4, 4,  "AND"},
			{NOTE_A,  4, 8,  ""},
			{NOTE_E,  5, 4,  "HURT"},
			{NOTE_D,  5, 2,  "YOU"},
			{0,       0, 4,  ""}
	};
};


#endif //ARDUINO_WOPR_JBWOPRTHERICKEFFECT_H
