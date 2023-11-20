// =============================================================================
//
// This is an example of how to use the display in the basic JBWoprDevice
// class.
//
// This example shows how to use the JBWoprDevice class to use the built in
// functions for the Defcon LED's. It also shows how to access the display
// directly using the Adafruit_LEDBackpack library. The display is exposed
// as an array of three Adafruit_AlphaNum4 instances.
//
// -----------------------------------------------------------------------------
//
// JBWopr Library - https://github.com/jonnybergdahl/Arduino_JBWopr_Library
// Copyright© 2023, Jonny Bergdahl
// MIT License
//
// =============================================================================

#include <Arduino.h>
#include <jbwopr.h>
#include <Adafruit_LEDBackpack.h>

// Define the segment names, this is missing in Adafruit's library. See the segment mapping
// here - https://learn.adafruit.com/14-segment-alpha-numeric-led-featherwing/usage
//
#define ALPHA_SEG_A  0b0000000000000001
#define ALPHA_SEG_B  0b0000000000000010
#define ALPHA_SEG_C  0b0000000000000100
#define ALPHA_SEG_D  0b0000000000001000
#define ALPHA_SEG_E  0b0000000000010000
#define ALPHA_SEG_F  0b0000000000100000
#define ALPHA_SEG_G1 0b0000000001000000
#define ALPHA_SEG_G2 0b0000000010000000
#define ALPHA_SEG_H  0b0000000100000000
#define ALPHA_SEG_J  0b0000001000000000
#define ALPHA_SEG_K  0b0000010000000000
#define ALPHA_SEG_L  0b0000100000000000
#define ALPHA_SEG_M  0b0001000000000000
#define ALPHA_SEG_N  0b0010000000000000
#define ALPHA_SEG_DP 0b0100000000000000

JBWoprDevice wopr;
std::array<Adafruit_AlphaNum4, 3> displays;

// Define a basic animation
uint16_t animation[] { 0,ALPHA_SEG_A,
					   1,ALPHA_SEG_H,
					   1,ALPHA_SEG_N,
					   2,ALPHA_SEG_L,
					   2,ALPHA_SEG_K,
					   3,ALPHA_SEG_A,
					   3,ALPHA_SEG_B,
					   3,ALPHA_SEG_C,
					   3,ALPHA_SEG_D,
					   2,ALPHA_SEG_N,
					   2,ALPHA_SEG_H,
					   1,ALPHA_SEG_K,
					   1,ALPHA_SEG_L,
					   0,ALPHA_SEG_D,
					   0,ALPHA_SEG_E,
					   0,ALPHA_SEG_F};

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

	// Get a pointer to the underlying array of displays
	displays = wopr.getDisplay();
}

void loop() {
	Serial.println("Display text");
	wopr.displayShowText("Left");
	delay(2000);

	Serial.println("Display right aligned text");
	wopr.displayShowText("Right", JBTextAlignment::RIGHT);
	delay(2000);

	Serial.println("Display centered text");
	wopr.displayShowText("Center", JBTextAlignment::CENTER);
	delay(2000);

	Serial.println("Scrolling text, default rate");
	wopr.displayScrollText("SCROLLING TEXT, Default rate");

	Serial.println("Scrolling text, 200 ms rate");
	wopr.displayScrollText("SCROLLING TEXT, Rate 200", 200);

	Serial.println("Scrolling text, 100 ms rate");
	wopr.displayScrollText("SCROLLING TEXT, Rate 100", 100);
	delay(2000);

	Serial.println("Changing brightness");
	wopr.displayShowText("BRIGHTNESS");
	for (uint8_t i = 100; i > 0; i--) {
		wopr.displaySetBrightness(i);
		delay(10);
	}
	for (uint8_t i = 0; i <= 100; i++) {
		wopr.displaySetBrightness(i);
		delay(10);
	}
	delay(2000);

	Serial.println("Display each digit in numeric order");
	runDisplayEachDigit();

	Serial.println("Run animation");
	runCustomAnimation();
	delay(2000);
}

// This is an example of how to use a display segment as standard Adafruit_AlphaNum4 display,
// part of the Adafruit_LEDBackPack library.
//
// This function displays segments one by one in numeric order.
void runDisplayEachDigit() {
	Adafruit_AlphaNum4 display = displays[1]; // Get the middle display
	wopr.displayClear();
	for (int i = 0; i < 15; i++)
	{
		display.writeDigitRaw(0, 1 << i);
		display.writeDigitRaw(1, 1 << i);
		display.writeDigitRaw(2, 1 << i);
		display.writeDigitRaw(3, 1 << i);
		display.writeDisplay();
		delay(400);
	}
}

// This is a more advanced example of how to use a display segment
//
// This function shows a basic animation
void runCustomAnimation() {
	Adafruit_AlphaNum4 display = displays[1]; // Use the middle display
	wopr.displayClear();
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < sizeof(animation)/2; j = j + 2)
		{
			display.clear();
			display.writeDigitRaw(animation[j], animation[j+1]);
			display.writeDisplay();
			delay(50);
		}
	}
	wopr.displayClear();
}
