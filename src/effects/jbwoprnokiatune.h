/// @file jbwoprnokiatune.h
/// @author Jonny Bergdahl
/// @brief Header file for the JBWoprNokiaTuneEffect.
/// @details Contains declarations for the JBWoprNokiaTuneEffect class.
/// This code is distributed under the MIT License. See LICENSE for details.
/// @date Created: 2023-10-02
/// @copyright Copyright© 2023, Jonny Bergdahl
///
#ifndef ARDUINO_WOPR_JBWOPRNOKIATUNE_H
#define ARDUINO_WOPR_JBWOPRNOKIATUNE_H

#include "jbwopreffects.h"

#define JBWOPR_EFFECT_NAME_NOKIA_TUNE "Nokia Tune"		///< Name of JBWoprNokiaTuneEffect

/// @brief Nokia tune effect
/// @details This class implements the Nokia tune effect.
class JBWoprNokiaTuneEffect : public JBWoprSongEffect {
public:
	/// @brief JBWoprNokiaTuneEffect constructor
	/// @ingroup EffectGroup
	/// @param woprBoard Pointer to JBWoprDevice
	/// @param tempo Tempo of effect
	/// @param duration Duration of effect
	/// @param name Name of effect
	explicit JBWoprNokiaTuneEffect(JBWoprDevice *woprBoard,
								   uint32_t tempo = 180,
								   uint32_t duration = -1,
								   const std::string name=JBWOPR_EFFECT_NAME_NOKIA_TUNE);

protected:
	// Hide these as they are hard coded
	virtual void setSong(std::vector<JBWoprSongEffect::Note>* song) {};
	virtual void setTempo(uint32_t tempo) {};

	/// @brief Nokia tune
	const std::vector<JBWoprSongEffect::Note> _nokiaTune {
			{ NOTE_E, 	5, 8, "      R    "},
			{ NOTE_D, 	5, 8, " O    R    " },
			{ NOTE_Fs, 	4, 4, " O    R  E " },
			{ NOTE_Gs, 	4, 4, " O I  R  EZ" },
			{ NOTE_Cs, 	5, 8, " O IA R  EZ" },
			{ NOTE_B, 	4, 8, "NO IA RU EZ" },
			{ NOTE_D, 	4, 4, "NO IA RULEZ" },
			{ NOTE_E, 	4, 4, "NOKIA RULEZ" },
			{ NOTE_B, 	4, 8, "" },
			{ NOTE_A, 	4, 8, "-" },
			{ NOTE_Cs, 	4, 4, "NOKIA RULEZ" },
			{ NOTE_E, 	4, 4, "-" },
			{ NOTE_A, 	4, 2, "NOKIA RULEZ" },
			{ 0, 	0, 4, "" }
	};
};


#endif //ARDUINO_WOPR_JBWOPRNOKIATUNE_H
