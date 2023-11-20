// =============================================================================
//
// This is an example of how to use audio in the basic JBWoprDevice class.
//
// This example shows how to use the JBWoprDevice class to play audio notes
// and tones.
//
// -----------------------------------------------------------------------------
//
// JBWopr Library - https://github.com/jonnybergdahl/Arduino_JBWopr_Library
// Copyright© 2023, Jonny Bergdahl
// MIT License
//
// =============================================================================
#include <Arduino.h>
#include <vector>
#include <jbwopr.h>

struct Note {
	note_t note;
	uint16_t octave;
	int16_t duration;
};

JBWoprDevice wopr;

std::vector<Note> melody = {
		// Nokia Ringtone
		// Original code at: https://github.com/robsoncouto/arduino-songs/blob/master/nokia/nokia.ino
		// Score available at https://musescore.com/user/29944637/scores/5266155
		{ NOTE_E, 5, 8 },
		{ NOTE_D, 5, 8 },
		{ NOTE_Fs, 4, 4 },
		{ NOTE_Gs, 4, 4 },
		{ NOTE_Cs, 5, 8 },
		{ NOTE_B, 4, 8 },
		{ NOTE_D, 4, 4 },
		{ NOTE_E, 4, 4 },
		{ NOTE_B, 4, 8 },
		{ NOTE_A, 4, 8 },
		{ NOTE_Cs, 4, 4 },
		{ NOTE_E, 4, 4 },
		{ NOTE_A, 4, 2 }
};

void setup() {
	// Setup serial
	//
	Serial.begin(115200);
	uint64_t timeout = millis() + 4000;
	while (!Serial && millis() < timeout)
	{
		delay(100);
	}

	// Setup the W.O.P.R.
	//
	Serial.println("=====================");
	Serial.println("Setting up WOPRDevice");
	wopr.begin(JBWoprBoardVariant::ORIGINAL);
}

void loop() {
	playNokiaTune();
	delay(500);

	playRandomTones();
	delay(500);

	sweepTones();
	delay(500);
}

void playNokiaTune() {
	Serial.println("Playing the Nokia tune");
	int notes = sizeof(melody) / sizeof(melody[0]) / 3;
	int tempo = 180;
	int wholenote = (60000 * 4) / tempo;
	int divider = 0, octave = 0, noteDuration = 0;

	for (int thisNote = 0; thisNote < melody.size(); thisNote++) {

		// calculates the duration of each note
		divider = melody[thisNote].duration;
		if (divider > 0) {
			// regular note, just proceed
			noteDuration = (wholenote) / divider;
		} else if (divider < 0) {
			// dotted notes are represented with negative durations!!
			noteDuration = (wholenote) / abs(divider);
			noteDuration *= 1.5; // increases the duration in half for dotted notes
		}
		wopr.audioPlayNote(melody[thisNote].note, melody[thisNote].octave);
		delay(noteDuration);
		wopr.audioClear();
	}
}

void playRandomTones() {
	Serial.println("Playing random tones");
	for (int i = 0; i < 50; i++) {
		wopr.audioPlayTone(random(90, 350));
		delay(80);
	}
	wopr.audioClear();
}

void sweepTones() {
	Serial.println("Sweep tones");
	for (int i = 100; i < 3500; i += 5) {
		wopr.audioPlayTone(i);
		delay(5);
	}
	wopr.audioClear();
}
