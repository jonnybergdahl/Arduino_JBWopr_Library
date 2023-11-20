// =============================================================================
//
// This is an example of how to use buttons with the basic JBWoprDevice class.
//
// This example shows how to use the JBWoprDevice class to use the button
// callbacks of the OneButton library.
//
// -----------------------------------------------------------------------------
//
// JBWopr Library - https://github.com/jonnybergdahl/Arduino_JBWopr_Library
// Copyright© 2023, Jonny Bergdahl
// MIT License
//
// =============================================================================
#include <jbwopr.h>

//#define BOARD_VERSION JBWoprBoardVariant::ORIGINAL
#define BOARD_VERSION JBWoprBoardVariant::HAXORZ

JBWoprDevice wopr;

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
	wopr.begin(BOARD_VERSION);

	// Set the WoprDevice button callbacks
	Serial.println("Set button callbacks");
	wopr.buttonFrontRightSetClickCallback(ButtonFrontRightClick);
	wopr.buttonFrontRightSetDoubleClickCallback(ButtonFrontRightDoubleClick);
	wopr.buttonFrontLeftSetClickCallback(ButtonFrontLeftClick);
	wopr.buttonFrontLeftSetDoubleClickCallback(ButtonFrontLeftDoubleClick);
	if (BOARD_VERSION == JBWoprBoardVariant::HAXORZ) {
		wopr.buttonBackTopSetClickCallback(ButtonBackTopClick);
		wopr.buttonBackTopSetDoubleClickCallback(ButtonBackTopDoubleClick);
		wopr.buttonBackBottomSetClickCallback(ButtonBackBottomClick);
		wopr.buttonBackBottomSetDoubleClickCallback(ButtonBackBottomDoubleClick);
	}

	// Now use the OneButton instance directly for more advanced stuff
	// We setup the front right button to respond to long press events
	OneButton* frontRightButton = wopr.getButtonFrontRight();
	frontRightButton->attachLongPressStop(ButtonFrontRightLongPressStart);
	frontRightButton->attachDuringLongPress(ButtonFrontRightDuringLongPress);
	frontRightButton->attachLongPressStop(ButtonFrontRightLongPressStop);
}

void loop() {
	wopr.loop();
}

void ButtonFrontRightClick() {
	Serial.println("Front right button clicked");
}

void ButtonFrontRightDoubleClick() {
	Serial.println("Front right button double clicked");
}

void ButtonFrontLeftClick() {
	Serial.println("Front left button clicked");
}

void ButtonFrontLeftDoubleClick() {
	Serial.println("Front left button double clicked");
}

void ButtonBackTopClick() {
	Serial.println("Back top button clicked");
}

void ButtonBackTopDoubleClick() {
	Serial.println("Back top button double clicked");
}

void ButtonBackBottomClick() {
	Serial.println("Back bottom button clicked");
}

void ButtonBackBottomDoubleClick() {
	Serial.println("Back bottom button double clicked");
}

void ButtonFrontRightLongPressStart() {
	Serial.println("Front right button long press Start");
}

void ButtonFrontRightDuringLongPress() {
	Serial.println("Front right button long press During");
}

void ButtonFrontRightLongPressStop() {
	Serial.println("Front right button long press Stop");
}
