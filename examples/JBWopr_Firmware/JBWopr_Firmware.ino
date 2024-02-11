// =============================================================================
//
// This is the full firmware sketch for the W.O.P.R device.
//
// At first boot, the device will start in AP mode and you can connect to it
// using the password "wopr1234". You will then be redirected to a captive
// portal where you can configure the device.
//
// In subsequent boots, the device will try to connect to the last used WiFi
// network. If it fails, it will start in AP mode again.
//
// Note: Uncomment code for the wanted firmware variant below.
//
// -----------------------------------------------------------------------------
//
// JBWopr Library - https://github.com/jonnybergdahl/Arduino_JBWopr_Library
// Copyright© 2023, Jonny Bergdahl
// MIT License
//
// =============================================================================
//
// Defines
// Uncomment one of the WoprDevice variants below.
//
//#define FIRMWARE_VARIANT_WIFI
//#define FIRMWARE_VARIANT_MQTT
#define FIRMWARE_VARIANT_HA
//
// Set the log level to LOG_LEVEL_TRACE in case you run into problems
#define LOG_LEVEL LogLevel::LOG_LEVEL_INFO

// Includes
#if defined(FIRMWARE_VARIANT_WIFI)
#include <jbwoprwifi.h>
#elif defined(FIRMWARE_VARIANT_MQTT)
#include <jbwoprmqtt.h>
#else
#include <jbwoprha.h>
#endif
#include <effects/jbwopreffects.h>
#include <effects/jbwoprnokiatune.h>
#include <effects/jbwoprtherickeffect.h>

// Declarations
#if defined(FIRMWARE_VARIANT_WIFI)
JBWoprWiFiDevice wopr;
#elif defined(FIRMWARE_VARIANT_MQTT)
JBWoprMqttDevice wopr;
#else
JBWoprHADevice wopr;
#endif
int32_t effectIndex = 0;					// Keeps track of the currently selected effect
std::vector<JBWoprEffectBase*> effects;		// List of all registered effects
bool showEffectName = false;				// Used to show the effect name when the effect stopped running

void displayEffectChanged() {
    // Stop the current effect
    wopr.effectsStopCurrentEffect();

    // Select the next effect in the list
    if (effectIndex >= effects.size()) {
        effectIndex = 0;
    }
    if (effectIndex < 0) {
        effectIndex = effects.size() - 1;
    }

    // Show the current effect name
    wopr.displayShowText(effects[effectIndex]->getName());
}

void buttonFrontRightClick() {
    Serial.println("Front right button clicked");
    wopr.effectsStartEffect(effects[effectIndex]);
}

void buttonFrontLeftClick() {
    Serial.println("Front left button clicked");
    effectIndex++;
    displayEffectChanged();
}

void buttonBackTopClick() {
    Serial.println("Back top button clicked");
    effectIndex++;
    displayEffectChanged();
}

void buttonBackBottomClick() {
    Serial.println("Back bottom button clicked");
    effectIndex--;
    displayEffectChanged();
}

// This method register all effects in the library
void registerEffects() {
    // Set Date/Time as the default effect
    wopr.effectsRegisterDefaultEffect(new JBWoprDateTimeDisplayEffect(&wopr));

    wopr.effectsRegisterEffect(new JBWoprXmasSecondsDisplayEffect(&wopr,
                                                                  200,
                                                                  0));
    wopr.effectsRegisterEffect(new JBWoprMissileCodeSolveEffect(&wopr,
                                                                CodeSolveVariant::MOVIE,
                                                                1000,
                                                                "Movie solve"));
    wopr.effectsRegisterEffect(new JBWoprMissileCodeSolveEffect(&wopr,
                                                                CodeSolveVariant::MESSAGE,
                                                                1000,
                                                                "Msg solve"));
    wopr.effectsRegisterEffect(new JBWoprMissileCodeSolveEffect(&wopr,
                                                                CodeSolveVariant::RANDOM,
                                                                1000,
                                                                "Random solve"));
    wopr.effectsRegisterEffect(new JBWoprDefconRainbowEffect(&wopr,
                                                             1000));
    wopr.effectsRegisterEffect(new JBWoprNokiaTuneEffect(&wopr,
                                                         180,
                                                         0));
    wopr.effectsRegisterEffect(new JBWoprTheRickEffect(&wopr));
    effects = wopr.effectsGetRegisteredEffects();
}

void setup() {
    // Setup serial
    Serial.begin(115200);

    // Wait for serial to be ready
    uint64_t timeout = millis() + 4000;
    while (!Serial && millis() < timeout)
    {
        delay(100);
    }

    Serial.println("=====================");
    Serial.println("Setting up WOPRDevice");
    // Set log level
    wopr.setLogLevel(LOG_LEVEL);

    // Load and get the list of registered effects
    registerEffects();

    // Initialize wopr
    wopr.begin(JBWoprBoardVariant::HAXORZ);

    // Attach button callback methods
    wopr.buttonFrontRightSetClickCallback(buttonFrontRightClick);
    wopr.buttonFrontLeftSetClickCallback(buttonFrontLeftClick);
    wopr.buttonBackTopSetClickCallback(buttonBackTopClick);
    wopr.buttonBackBottomSetClickCallback(buttonBackBottomClick);

    // Show instructions
    wopr.displayScrollText("Left - Select effect, Right - Run effect");
    Serial.println("Use back top/front left button to select next effect");
    Serial.println("Use back bottom button to select previous effect");
    Serial.println("Use front right button to start selected effect");

    // Display first choice
    wopr.displayShowText(effects[effectIndex]->getName());
    Serial.println("Setup done");
}

void loop() {
    // Run the device loop
    wopr.loop();

    if (wopr.effectsCurrentEffectIsRunning() && !showEffectName) {
        // Show the effect name when the effect stopped running
        showEffectName = true;
    }
    if (!wopr.effectsCurrentEffectIsRunning() && showEffectName) {
        // Show the effect name
        showEffectName = false;
        wopr.displayShowText(effects[effectIndex]->getName());
    }
}