// =============================================================================
//
// This is an example of how to use the basic JBWoprDevice class.
//
// This example shows how to use the JBWoprDevice class to create a simple
// device that can display text and scroll text on the display.
//
// -----------------------------------------------------------------------------
//
// JBWopr Library - https://github.com/jonnybergdahl/Arduino_JBWopr_Library
// Copyright© 2023, Jonny Bergdahl
// MIT License
//
// =============================================================================
#include <vector>
#include <time.h>
#include <WiFi.h>
#include <jbwopr.h>
#include <jbwoprhelpers.h>
#include "effects/jbwopreffects.h"
#include "secrets.h"

#define BOARD_VERSION JBWoprBoardVariant::ORIGINAL

JBWoprDevice wopr;
uint32_t connectionTimeout = 10000;
std::vector<JBWoprEffectBase*> effects;
int32_t currentEffect = 0;
bool resetEffect = false;

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

	// Since we have no stored config, we set it up here
	JBWoprConfiguration* config = wopr.getConfiguration();
	config->displayBrightness = 50;   // Half brightness
	config->defconLedsBrightness = 50;  // Half brightness
	config->timeFormat = "%H.%M.%S";  // 14.23.59 <- Note: the display can't really show a ":" character.
	config->dateFormat = "%Y-%m-%d";  // 2023-09-26
	config->effectsTimeout = 2;

	// We are setting log level to max log level
	wopr.getLogger()->setLogLevel(LogLevel::LOG_LEVEL_TRACE);

	// Setting up the devicee
	wopr.begin(BOARD_VERSION);

	// Now hook up the front buttons
	OneButton* left = wopr.getButtonFrontLeft();
	left->attachClick(onLeftClick);
	OneButton* right = wopr.getButtonFrontRight();
	right->attachClick(onRightClick);

	wopr.displayShowText("W.O.P.R", JBTextAlignment::CENTER);
	delay(1000);

	// Since some effects needs the current time, we need to connect to WiFi
	Serial.print("Connecting to WiFi");
	wopr.displayShowText("Connect", JBTextAlignment::CENTER);
	WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
	uint32_t startTime = millis();
	while (WiFi.status() != WL_CONNECTED) {
		if (millis() - startTime >= connectionTimeout) {
			Serial.println();
			Serial.println("Connection timeout");
			wopr.displayShowText("WiFi Failed", JBTextAlignment::CENTER);
			delay(2000);
			break;
		}
		delay(100);
		Serial.print(".");
	}
	Serial.println();
	if (WiFi.status() == WL_CONNECTED) {
		Serial.println("Connected to WiFi");
		wopr.displayShowText("Connected", JBTextAlignment::CENTER);
		delay(1000);

		// Use the JBTimeHelper class to get the local time zone and get the local time
		wopr.displayShowText("Get time", JBTextAlignment::CENTER);
		tm timeinfo;
		if (!JBTimeHelper::getTime(&timeinfo)) {
			Serial.println("Failed to obtain time");
			wopr.displayShowText("Time failed", JBTextAlignment::CENTER);
		}
	}
	delay(1000);

	// Create effects
	Serial.println("Create effects");
	setupEffects();

	Serial.println("Done, use buttons to switch between effects");
}

void loop() {
	// Call the wopr.loop() function here to let it do it's magic
	wopr.loop();

	// Check if we switched effects with the buttons
	if (resetEffect) {
		Serial.print("currentEffect: ");
		Serial.println(currentEffect);
		wopr.effectStopCurrent();
		resetEffect = false;
		startEffect();
	}

	// If we are not running, just display a "pause" text
	if (!wopr.effectIsRunning())
	{
		wopr.displayShowText("<- SWITCH ->");
	}
}

void onRightClick() {
	Serial.println("Next effect");
	currentEffect++;
	if (currentEffect > effects.size() -1)
	{
		currentEffect = 0;
	}
	resetEffect = true;
}

void onLeftClick() {
	Serial.println("Previous effect");
	currentEffect--;
	if (currentEffect < 0)
	{
		currentEffect = effects.size() - 1;
	}
	resetEffect = true;
}

void startEffect() {
	JBWoprEffectBase* effect = effects[currentEffect];
	uint32_t duration = 10000;

	Serial.printf("Running effect %i: %s for %i ms\n", currentEffect, effect->getName().c_str(), duration);
	wopr.displayShowText(effect->getName());
	delay(1000);
	wopr.effectStart(effect, duration);
}

void setupEffects() {
	// Now we create all effects and put them in a vector
	// Simple text display
	JBWoprTextDisplayEffect* textDisplay = new JBWoprTextDisplayEffect(&wopr);
	textDisplay->setText("SOME TEXT");
	effects.push_back(textDisplay);

	// Centered text display
	textDisplay = new JBWoprTextDisplayEffect(&wopr, JBTextAlignment::CENTER);
	textDisplay->setText("CENTERED");
	effects.push_back(textDisplay);

	// Scrolling text effect
	JBWoprScrollTextDisplayEffect* scrollTextDisplay = new JBWoprScrollTextDisplayEffect(&wopr);
	scrollTextDisplay->setText("This is some scrolling text that is longer than 12 characters");
	effects.push_back(scrollTextDisplay);

	// Clock display, time only
	JBWoprTimeDisplayEffect* timeEffect = new JBWoprTimeDisplayEffect(&wopr);
	effects.push_back(timeEffect);

	// Clock display, date only
	JBWoprDateDisplayEffect* dateEffect = new JBWoprDateDisplayEffect(&wopr);
	effects.push_back(dateEffect);

	// Clock display, time and date
	JBWoprDateTimeDisplayEffect* dateTimeEffect = new JBWoprDateTimeDisplayEffect(&wopr);
	effects.push_back(dateTimeEffect);

	// Seconds until Xmas
	JBWoprXmasSecondsDisplayEffect* xmasSecondsDisplay = new JBWoprXmasSecondsDisplayEffect(&wopr);
	effects.push_back(xmasSecondsDisplay);

	// Missile solve effect, movie version
	JBWoprMissileCodeSolveEffect* missileCodeSolve1 = new JBWoprMissileCodeSolveEffect(&wopr);
	effects.push_back(missileCodeSolve1);

	// Missile solve effect, message version
	JBWoprMissileCodeSolveEffect* missileCodeSolve2 = new JBWoprMissileCodeSolveEffect(&wopr);
	missileCodeSolve2->setCodeSolveVariant(CodeSolveVariant::MESSAGE);
	effects.push_back(missileCodeSolve2);

	// Missile solve effect, random version
	JBWoprMissileCodeSolveEffect* missileCodeSolve3 = new JBWoprMissileCodeSolveEffect(&wopr);
	missileCodeSolve2->setCodeSolveVariant(CodeSolveVariant::RANDOM);
	effects.push_back(missileCodeSolve3);

	// Defcon LED rainbow effect
	JBWoprDefconRainbowEffect* rainbowEffect = new JBWoprDefconRainbowEffect(&wopr);
	effects.push_back(rainbowEffect);

	// The Rick effect
	JBWoprTheRickEffect* theRick = new JBWoprTheRickEffect(&wopr);
	effects.push_back(theRick);
}
