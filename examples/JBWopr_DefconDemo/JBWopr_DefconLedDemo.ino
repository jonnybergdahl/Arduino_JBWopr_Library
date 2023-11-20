// =============================================================================
//
// This is an example of how to use the Defcon LED's in the basic JBWoprDevice
// class.
//
// This example shows how to use the JBWoprDevice class to use the built in
// functions for the Defcon LED's. It also shows how to access the LED's
// directly using the Adafruit_Neopixel library
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
#include <Adafruit_NeoPixel.h>

JBWoprDevice wopr;
Adafruit_NeoPixel* strip;

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

	// Get a pointer to the underlying NeoPixel strip
	strip = wopr.getDefconLeds();
}

void loop() {
	Serial.println("Setting all to Red");
	wopr.defconLedsSetColor(0xFF0000);
	delay(1000);
	Serial.println("Setting all to Green");
	wopr.defconLedsSetColor(0x00F000);
	delay(1000);
	Serial.println("Setting all to Blue");
	wopr.defconLedsSetColor(0x0000FF);
	delay(1000);


	Serial.println("Turning them on one after another");
	wopr.defconLedsClear();
	wopr.defconLedSetColor(JBDefconLevel::DEFCON_1, 0xFF0000);
	delay(500);
	wopr.defconLedSetColor(JBDefconLevel::DEFCON_2, 0xFFFF00);
	delay(500);
	wopr.defconLedSetColor(JBDefconLevel::DEFCON_3, 0x00FF00);
	delay(500);
	wopr.defconLedSetColor(JBDefconLevel::DEFCON_4, 0x00FFFF);
	delay(500);
	wopr.defconLedSetColor(JBDefconLevel::DEFCON_5, 0x0000FF);
	delay(500);

	wopr.defconLedSetColor(JBDefconLevel::DEFCON_1, 0);
	delay(500);
	wopr.defconLedSetColor(JBDefconLevel::DEFCON_2, 0);
	delay(500);
	wopr.defconLedSetColor(JBDefconLevel::DEFCON_3, 0);
	delay(500);
	wopr.defconLedSetColor(JBDefconLevel::DEFCON_4, 0);
	delay(500);
	wopr.defconLedSetColor(JBDefconLevel::DEFCON_5, 0);
	delay(500);

	Serial.println("Setting DEFCON 5");
	wopr.defconLedsSetDefconLevel(JBDefconLevel::DEFCON_5);
	delay(1000);

	Serial.println("Setting DEFCON 4");
	wopr.defconLedsSetDefconLevel(JBDefconLevel::DEFCON_4);
	delay(1000);

	Serial.println("Setting DEFCON 3");
	wopr.defconLedsSetDefconLevel(JBDefconLevel::DEFCON_3);
	delay(1000);

	Serial.println("Setting DEFCON 2");
	wopr.defconLedsSetDefconLevel(JBDefconLevel::DEFCON_2);
	delay(1000);

	Serial.println("Setting DEFCON 1");
	wopr.defconLedsSetDefconLevel(JBDefconLevel::DEFCON_1);
	delay(1000);

	Serial.println("Setting DEFCON None");
	wopr.defconLedsSetDefconLevel(JBDefconLevel::DEFCON_NONE);
	delay(1000);

	Serial.println("Changing brightness");
	wopr.defconLedsSetColor(0x0000FF);
	for (uint32_t i = 100; i > 0; i--) {
		wopr.defconLedsSetBrightness(i);
		delay(10);
	}
	for (uint32_t i = 0; i < 100; i++) {
		wopr.defconLedsSetBrightness(i);
		delay(10);
	}
	delay(2000);

	Serial.println("Theater chase");
	theaterChase(0x00ff00, 150); // White
	delay(2000);
}

// This is an example of how to use the Defcon LED's as a standard Adafruit_Neopixel strip.
//
// The code is copied directly from the Adafruit_Neopixel library example strandtest_wheel.ino
//
//	Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait) {
	for (int j=0; j<10; j++) {  //do 10 cycles of chasing
		for (int q=0; q < 3; q++) {
			for (uint16_t i=0; i < strip->numPixels(); i=i+3) {
				strip->setPixelColor(i+q, c);    //turn every third pixel on
			}
			strip->show();

			delay(wait);

			for (uint16_t i=0; i < strip->numPixels(); i=i+3) {
				strip->setPixelColor(i+q, 0);        //turn every third pixel off
			}
		}
	}
}
