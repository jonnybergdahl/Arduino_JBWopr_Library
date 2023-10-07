/***************************************************
  War Games - W.O.P.R. Missile Codes
  Original code 2023 UNexpected Maker
  Refactored by Jonny Bergdahl to use the Arduino_WOPR library

  Licensed under MIT Open Source

  This code is designed specifically to run on an ESP32. It uses features only
  available on the ESP32 like RMT and ledc.

  W.O.P.R is available here...

  https://unexpectedmaker.com/shop/wopr-missile-launch-code-display-kit
  https://www.tindie.com/products/seonr/wopr-missile-launch-code-display-kit/

  Wired up for use with the TinyPICO, TinyS2 or TinyS3 Development Boards & TinyPICO Analog Audio Shield

  All products also available on my own store

  http://unexpectedmaker.com/shop/

 ***************************************************/

#include "FS.h"
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <Adafruit_GFX.h>
#include <Adafruit_LEDBackpack.h>
#include <Adafruit_NeoPixel.h>
#include <OneButton.h>
#include <WiFi.h>
#include "time.h"
#include "secret.h"
#include <Arduino.h>
#include "utc_offset.h"
#include <wopr.h>

// Defines
#ifndef _BV
#define _BV(bit) (1 << (bit))
#endif

#define ELEMENTS(x) (sizeof(x) / sizeof(x[0]))


// NTP Wifi Time
const char* ntpServer = "pool.ntp.org";
bool didChangeClockSettings = false;
bool hasWiFi = false;
bool isFirstBoot = false;

//// Program & Menu state
String clockSeparators[] = { " ", "-", "_" };
String stateStrings[] = { "MENU", "RUNNING", "SETTINGS" };
String menuStrings[] = { "MODE MOVIE", "MODE RANDOM", "MODE MESSAGE", "MODE CLOCK", "SETTINGS" };
String settingsStrings[] = { "GMT ", "24H MODE ", "BRIGHT ", "CLK RGB ", "CLK CNT ", "CLK SEP ", "UPDATE GMT" };

enum states {
    MENU = 0,
    RUNNING = 1,
    SET = 2,
} currentState;

enum modes {
    MOVIE = 0,
    RANDOM = 1,
    MESSAGE = 2,
    CLOCK = 3,
    SETTINGS = 4,
} currentMode;

enum settings {
    SET_GMT = 0,
    SET_24H = 1,
    SET_BRIGHT = 2,
    SET_CLOCK_RGB = 3,
    SET_CLOCK = 4,
    SET_SEP = 5,
    SET_UPDATE_GMT = 6,
} currentSetting;

struct Settings {
    uint32_t clockCountdownTime;
    uint8_t timeOffset;
    bool use24HourFormat;
    uint32_t defconLedsBrightness;
    uint8_t displayBrightness;
    uint8_t separatorCharacter;
};

Settings config {
        60,
        0,
        true,
        50,
        15,
        0  // 0 is " ", 1 is "-", 2 is "_"
};

/* Code cracking stuff
   Though this works really well, there are probably much nicer and cleaner
   ways of doing this, so feel free to improve it and make a pull request!
*/
uint8_t counter = 0;
unsigned long nextTick = 0;
unsigned long nextSolve = 0;
uint16_t tickStep = 100;
uint16_t solveStep = 1000;
uint16_t solveStepMin = 4000;
uint16_t solveStepMax = 8000;
float solveStepMulti = 1;
uint8_t solveCount = 0;
uint8_t solveCountFinished = 10;
byte lastDefconLevel = 0;

// Audio stuff
bool beeping = false;
unsigned long nextBeep = 0;
uint8_t beepCount = 3;
int freq = 2000;
int channel = 0;
int resolution = 8;

// RGB stuff
unsigned long nextRGB = 0;
long nextPixelHue = 0;

// General stuff
unsigned long countdownToClock = 0;


char displaybuffer[12] = { '-', '-', '-', ' ', '-', '-', '-', '-', ' ', '-', '-', '-' };
char missile_code[12] = { 'A', 'B', 'C', 'D', 'E', 'F', '0', '1', '2', '3', '4', '5' };
char missile_code_movie[12] = { 'C', 'P', 'E', ' ', '1', '7', '0', '4', ' ', 'T', 'K', 'S' };
char missile_code_message[12] = { 'L', 'O', 'L', 'Z', ' ', 'F', 'O', 'R', ' ', 'Y', 'O', 'U' };
uint8_t code_solve_order_movie[10] = { 7, 1, 4, 6, 11, 2, 5, 0, 10, 9 };  // 4 P 1 0 S E 7 C K T
uint8_t code_solve_order_random[12] = { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 };

WOPRBoard wopr;

void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("");
    Serial.println("Wargames Missile Codes");

    if (!LittleFS.begin(true)) {
        Serial.println("LittleFS Mount Failed");
        return;
    }

    wopr.begin(WOPRBoardVersion::HAXORZ);

    // Attach button IO for OneButton
    wopr.buttonFrontRight.attachClick(ButtonFrontRightPress);
    wopr.buttonFrontRight.attachDuringLongPress(ButtonFrontRightLongPress);
    wopr.buttonFrontLeft.attachClick(ButtonFrontLeftPress);
    if (wopr.wopr_version == WOPRBoardVersion::HAXORZ) {
        wopr.buttonBackTop.attachClick(ButtonBackTopPress);
        wopr.buttonBackBottom.attachClick(ButtonBackTopPress);
    }
    // Reset the code variables
    ResetCode();


    wopr.displaySetText("LOAD CONFIG");
    loadSettings();

    wopr.displaySetBrightness(config.displayBrightness);
    wopr.defconLeds.setBrightness(config.defconLedsBrightness);

    /* Initialise WiFi to get the current time.
       Once the time is obtained, the internal ESP32 RTC is used to keep the time
       Make sure you have set your SSID and Password in secret.h
    */

    wopr.displaySetText("START WIFI");
    StartWifi();

    // User settable countdown from main menu to go into clock if no user interaction
    // Has happened. settings_clockCountdownTime is in seconds and we want milliseconds

    countdownToClock = millis() + config.clockCountdownTime * 1000;

    // Display MENU
    wopr.displaySetText("MENU");
}

void UpdateGMT_NTP() {
    wopr.displaySetText("Getting GMT");

    // IP Adddress Stuff
    char szIP[32];

    if (GetExternalIP(szIP)) {
        int iTimeOffset;  // offset in seconds
        // Serial.print("My IP: ");
        // Serial.println(szIP);
        // Get our time zone offset (including daylight saving time)
        iTimeOffset = GetTimeOffset(szIP);
        if (iTimeOffset != -1) {
            //init and get the time
            config.timeOffset = iTimeOffset / 3600;
            saveSettings();
        } else {
            Serial.println("*** TZ info failed");
        }
    } else {
        Serial.println("*** IP info failed");
    }
}

void StartWifi() {

    if (ssid == "PUT SSID HERE") {
        wopr.displaySetText("SSID NOT SET");
        wopr.defconLedsSetColor(Adafruit_NeoPixel::Color(255, 0, 0));
        hasWiFi = false;
        delay(2000);
    } else if (password == "PUT PASSWORD HERE") {
        wopr.displaySetText("PASS NOT SET");
        wopr.defconLedsSetColor(Adafruit_NeoPixel::Color(255, 0, 0));
        hasWiFi = false;
        delay(2000);
    } else {
        wopr.displaySetText("TRYING WiFi");

        //connect to WiFi
        int wifi_counter = 100;
        Serial.printf("Connecting to %s ", ssid);
        WiFi.begin(ssid, password);
        while (WiFi.status() != WL_CONNECTED && wifi_counter > 0) {
            delay(100);
            RGB_Rainbow(0);
            wifi_counter--;
            Serial.print(".");
        }

        if (WiFi.status() != WL_CONNECTED && wifi_counter == 0) {
            wopr.displaySetText("WiFi FAILED");
            wopr.defconLedsSetColor(Adafruit_NeoPixel::Color(255, 0, 0));
            hasWiFi = false;
            //while(1) {delay(1000);}
            delay(3000);
        } else {
            Serial.println(" CONNECTED");
            wopr.displaySetText("WiFi GOOD");
            wopr.defconLedsSetColor(Adafruit_NeoPixel::Color(0, 255, 0));

            hasWiFi = true;
            if (isFirstBoot)  // Need to grab from online and save locally if this is out first boot
            {
                isFirstBoot = false;
                UpdateGMT_NTP();
            }

            configTime(config.timeOffset * 3600, 0, ntpServer);

            delay(100);

            struct tm timeinfo;
            if (!getLocalTime(&timeinfo)) {
                Serial.println("Failed to obtain time");
                wopr.displaySetText("Time FAILED");
                wopr.defconLedsSetColor(Adafruit_NeoPixel::Color(255, 0, 0));
            } else {
                Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");

                wopr.displaySetText("Time Set OK");
                wopr.defconLedsSetColor(Adafruit_NeoPixel::Color(0, 0, 255));
            }

            delay(1500);

            wopr.displayScrollText( GetSecondsUntilXmas(), 200 );
        }
    }
}

// Button press code her
long nextButtonPress = 0;

// This is triggered from a long press on button 1
void ButtonFrontRightLongPress() {
    Serial.println("Long press");
    if (currentState == SET && currentMode == SETTINGS) {
        Serial.println("SAAAAAVE!");
        wopr.displaySetText("SAVING...");
        saveSettings();
        delay(500);

        if (didChangeClockSettings) {
            // If the clock parameters were changed, we need to re-set the ESP32 RTC time.
            configTime(config.timeOffset * 3600, 0, ntpServer);
        }

        // Reset the menu state after save
        currentState = MENU;
        currentSetting = SET_GMT;
        countdownToClock = millis() + config.clockCountdownTime * 1000;
        wopr.displaySetText("SETTINGS");
    }
}

void ButtonFrontRightPress() {
    // Only allow a button press every 10ms
    if (nextButtonPress < millis()) {
        nextButtonPress = millis() + 10;

        // If we are not in the menu, cancel the current state and show the menu
        if (currentState == RUNNING) {
            currentState = MENU;

            wopr.displaySetText("MENU");

            //Shutdown the audio if it's beeping
            wopr.audioPlayTone(0);
            beeping = false;
        } else if (currentState == MENU) {
            // Update the current program state and display it on the menu
            int nextMode = (int)currentMode + 1;
            if (nextMode == ELEMENTS(menuStrings))
                nextMode = 0;
            currentMode = (modes)nextMode;

            wopr.displaySetText(menuStrings[(int)currentMode]);
        } else if (currentState == SET) {
            // Update the current settings state and display it on the menu
            int nextMode = (int)currentSetting + 1;
            if (nextMode == ELEMENTS(settingsStrings))
                nextMode = 0;
            currentSetting = (settings)nextMode;

            ShowSettings();
        }

        // Reset the clock countdown now that we are back in the menu
        // settings_clockCountdownTime is in seconds, we need milliseconds
        countdownToClock = millis() + config.clockCountdownTime * 1000;

        Serial.print("Current State: ");
        Serial.print(stateStrings[(int)currentState]);

        Serial.print("  Current Mode: ");
        Serial.println(menuStrings[(int)currentMode]);
    }
}

void ButtonFrontLeftPress() {
    // Only allow a button press every 10ms
    if (nextButtonPress < millis()) {
        nextButtonPress = millis() + 10;

        // If in the menu, start whatever menu option we are in
        if (currentState == MENU) {
            // Check to see what mode we are in, because not all modes start the
            // code sequence is
            if (currentMode == SETTINGS) {
                currentState = SET;
                Serial.println("Going into settings mode");
                ShowSettings();
            } else {
                // Set the defcon state if we are not the clock, otherwise clear the RGB
                if (currentMode != CLOCK)
                    wopr.defconSetLevel(DefconLevel::DEFCON_5);
                else
                    wopr.defconLeds.clear();

                ResetCode();
                wopr.displayClear();
                currentState = RUNNING;
            }
        } else if (currentState == SET) {
            // If in the settings, cycle the setting for whatever setting we are on
            if (currentMode == SETTINGS) {
                UpdateSetting(1);
            }
        }
    }

    Serial.print("Current State: ");
    Serial.println(stateStrings[(int)currentState]);
}

void ButtonBackTopPress() {
    // Only allow a button press every 10ms
    if (nextButtonPress < millis()) {
        nextButtonPress = millis() + 10;

        // If in the settings, cycle the setting for whatever menu option we are in
        if (currentState == SET && currentMode == SETTINGS) {
            UpdateSetting(1);
        }
    }
}

void ButtonBackBottomPress() {
    // Only allow a button press every 10ms
    if (nextButtonPress < millis()) {
        nextButtonPress = millis() + 10;

        // If in the settings, cycle the setting for whatever menu option we are in
        if (currentState == SET && currentMode == SETTINGS) {
            UpdateSetting(-1);
        }
    }
}

// Cycle the setting for whatever current setting we are changing
void UpdateSetting(int dir) {
    if (currentSetting == SET_GMT) {
        config.timeOffset += dir;
        if (config.timeOffset > 14)
            config.timeOffset = -12;
        else if (config.timeOffset < -12)
            config.timeOffset = 14;

        didChangeClockSettings = true;
    } else if (currentSetting == SET_24H) {
        config.use24HourFormat = !config.use24HourFormat;
    } else if (currentSetting == SET_CLOCK_RGB) {
        config.defconLedsBrightness += (dir * 10);
        if (config.defconLedsBrightness > 250)
            config.defconLedsBrightness = 0;
        else if (config.defconLedsBrightness < 0)
            config.defconLedsBrightness = 250;
    } else if (currentSetting == SET_BRIGHT) {
        config.displayBrightness += dir;
        if (config.displayBrightness > 15)
            config.displayBrightness = 0;
        else if (config.displayBrightness < 0)
            config.displayBrightness = 15;

        wopr.displaySetBrightness(config.displayBrightness);
    } else if (currentSetting == SET_CLOCK) {
        config.clockCountdownTime += dir * 10;  // Larger increments for quicker change
        if (config.clockCountdownTime > 60)
            config.clockCountdownTime = 0;
        else if (config.clockCountdownTime < 0)
            config.clockCountdownTime = 60;

        countdownToClock = millis() + config.clockCountdownTime * 1000;
    } else if (currentSetting == SET_SEP) {
        config.separatorCharacter += dir;
        if (config.separatorCharacter == 3)
            config.separatorCharacter = 0;
        else if (config.separatorCharacter < 0)
            config.separatorCharacter = 2;
    } else if (currentSetting == SET_UPDATE_GMT) {
        UpdateGMT_NTP();
        wopr.displaySetText("GMT now " + String(config.timeOffset));
        configTime(config.timeOffset * 3600, 0, ntpServer);
        delay(2000);
        wopr.displayClear();
        currentMode = CLOCK;
        currentState = RUNNING;

        if (config.defconLedsBrightness == 0)
            wopr.defconLeds.clear();
        else
            wopr.defconLeds.setBrightness(config.defconLedsBrightness);

        return;
    }

    // Update the display showing whatever the new current setting is
    ShowSettings();
}

void ShowSettings() {
    Serial.print("current setting: ");
    Serial.println(currentSetting);

    String val = "";

    if (currentSetting == SET_GMT)
        val = String(config.timeOffset);
    else if (currentSetting == SET_24H)
        val = config.use24HourFormat ? "ON" : "OFF";
    else if (currentSetting == SET_BRIGHT)
        val = String(config.displayBrightness);
    else if (currentSetting == SET_CLOCK) {
        if (config.clockCountdownTime > 0)
            val = String(config.clockCountdownTime);
        else
            val = "OFF";
    } else if (currentSetting == SET_CLOCK_RGB) {
        if (config.defconLedsBrightness > 0)
            val = String(int((float)config.defconLedsBrightness / 250.0 * 100)) + "%";
        else
            val = "OFF";
    } else if (currentSetting == SET_SEP) {
        if (config.separatorCharacter == 0)
            val = "SPC";
        else
            val = clockSeparators[config.separatorCharacter];
    } else if (currentSetting == SET_UPDATE_GMT) {
        val = "";
    }

    wopr.displaySetText(settingsStrings[(int)currentSetting] + val);
}

// Adjust the LED display brightness: Range is 0-15

// Take the time data from the RTC and format it into a string we can display
void DisplayTime() {
    if (!hasWiFi) {
        wopr.displaySetText("NO CLOCK");
        wopr.defconLedsSetColor(Adafruit_NeoPixel::Color(0, 255, 0));
        return;
    }
    // Store the current time into a struct
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        Serial.println("Failed to obtain time");
        wopr.displaySetText("TIME FAILED");
        wopr.defconLedsSetColor(Adafruit_NeoPixel::Color(0, 255, 0));
        return;
    }
    // Formt the contents of the time struct into a string for display
    char DateAndTimeString[12];
    String sep = clockSeparators[config.separatorCharacter];

    int the_hour = timeinfo.tm_hour;

    // Adjust for 24 hour display mode
    if (!config.use24HourFormat && the_hour > 12)
        the_hour -= 12;

    // Padd the time if the hour is a single digit
    if (the_hour < 10)
        sprintf(DateAndTimeString, "   %d%s%02d%s%02d", the_hour, sep, timeinfo.tm_min, sep, timeinfo.tm_sec);
    else
        sprintf(DateAndTimeString, "  %d%s%02d%s%02d", the_hour, sep, timeinfo.tm_min, sep, timeinfo.tm_sec);

    // Iterate through each digit on the display and populate the time, or clear the digit
    uint8_t curDisplay = 0;
    uint8_t curDigit = 0;

    for (uint8_t i = 0; i < 10; i++) {
        wopr.display[curDisplay].writeDigitAscii(curDigit, DateAndTimeString[i]);
        curDigit++;
        if (curDigit == 4) {
            curDigit = 0;
            curDisplay++;
        }
    }

    // Show whatever is in the display buffer on the display
    wopr.displayShow();
}

// Display whatever is in txt on the display


// Return a random time step for the next solving solution
uint16_t GetNextSolveStep() {
    return random(solveStepMin, solveStepMax) * solveStepMulti;
}

// Fill whatever is in the code buffer into the display buffer
void FillCodes() {
    int matrix_index = 0;
    int character_index = 0;
    char c = 0;
    char c_code = 0;

    for (int i = 0; i < 12; i++) {
        c = displaybuffer[i];
        c_code = missile_code[i];
        if (c == '-') {
            // c is a character we need to randomise
            c = random(48, 91);
            while ((c > 57 && c < 65) || c == c_code)
                c = random(48, 91);
        }
        wopr.display[matrix_index].writeDigitAscii(character_index, c);
        character_index++;
        if (character_index == 4) {
            character_index = 0;
            matrix_index++;
        }
    }

    // Show whatever is in the display buffer on the display
    wopr.displayShow();
}

// Randomise the order of the code being solved
void RandomiseSolveOrder() {
    // Ensure all array slots start with 99
    for (uint8_t i = 0; i < 12; i++)
        code_solve_order_random[i] = 99;

    for (uint8_t i = 0; i < 12; i++) {
        uint8_t ind = random(0, 12);
        while (code_solve_order_random[ind] < 99)
            ind = random(0, 12);

        code_solve_order_random[ind] = i;
    }
}

// Reset the code being solved back to it's starting state
void ResetCode() {
    if (currentMode == MOVIE) {
        solveStepMulti = 1;
        solveCountFinished = 10;
        for (uint8_t i = 0; i < 12; i++)
            missile_code[i] = missile_code_movie[i];
    } else if (currentMode == RANDOM) {
        solveStepMulti = 0.5;

        // Randomise the order in which we solve this code
        RandomiseSolveOrder();

        // Set the code length and populate the code with random chars
        solveCountFinished = 12;

        for (uint8_t i = 0; i < 12; i++) {
            Serial.print("Setting code index ");
            Serial.print(i);

            // c is a character we need to randomise
            char c = random(48, 91);
            while (c > 57 && c < 65)
                c = random(48, 91);


            Serial.print(" to char ");
            Serial.println(c);

            missile_code[i] = c;
        }
    } else if (currentMode == MESSAGE) {
        solveStepMulti = 0.5;

        // Randomise the order in which we solve this code
        RandomiseSolveOrder();

        // Set the code length and populate the code with the stored message
        solveCountFinished = 12;
        for (uint8_t i = 0; i < 12; i++)
            missile_code[i] = missile_code_message[i];
    }

    // Set the first solve time step for the first digit lock

    solveStep = GetNextSolveStep();
    nextSolve = millis() + solveStep;
    solveCount = 0;
    lastDefconLevel = 0;

    // Clear code display buffer
    for (uint8_t i = 0; i < 12; i++) {
        if (currentMode == 0 && (i == 3 || i == 8))
            displaybuffer[i] = ' ';
        else
            displaybuffer[i] = '-';
    }
}

/*  Solve the code based on the order of the solver for the current mode
    This is fake of course, but so was the film!
    The reason we solve based on a solver order, is so we can solve the code
    in the order it was solved in the movie.
*/

void SolveCode() {
    // If the number of digits solved is less than the number to be solved
    if (solveCount < solveCountFinished) {
        // Grab the next digit from the code based on the mode
        uint8_t index = 0;

        if (currentMode == MOVIE) {
            index = code_solve_order_movie[solveCount];
            displaybuffer[index] = missile_code[index];
        } else {
            index = code_solve_order_random[solveCount];
            displaybuffer[index] = missile_code[index];
        }

        Serial.println("Found " + String(displaybuffer[index]) + " @ index: " + String(solveCount));

        // move tghe solver to the next digit of the code
        solveCount++;

        // Get current percentage of code solved so we can set the defcon display
        float solved = 1 - ((float)solveCount / (float)solveCountFinished);

        Serial.println("Solved " + String(solved));

        DefconLevel defconValue = DefconLevel(int(solved * 5 + 1));
        wopr.defconSetLevel(defconValue);

        Serial.println("Defcon " + String(defconValue));

        Serial.println("Next solve index: " + String(solveCount));

        FillCodes();

        // Long beep to indicate a digit in he code has been solved!
        wopr.audioPlayTone(1500);
        beeping = true;
        beepCount = 3;
        nextBeep = millis() + 500;
    }
}

void RGB_Rainbow(int wait) {
    if (nextRGB < millis()) {
        nextRGB = millis() + wait;
        nextPixelHue += 256;

        if (nextPixelHue > 65536)
            nextPixelHue = 0;

        // For each RGB LED
        for (int i = 0; i < 5; i++) {
            int pixelHue = nextPixelHue + (i * 65536L / 5);
            wopr.defconLeds.setPixelColor(i, Adafruit_NeoPixel::gamma32(Adafruit_NeoPixel::ColorHSV(pixelHue)));
        }
        wopr.defconLeds.show();
    }
}

int pingpong(int t, int length) {
    return t % length;
}

void loop() {
    wopr.loop();

    // We are in the menu
    if (currentState == MENU || currentState == SET) {
        // We dont need to do anything here, but lets show some fancy RGB!
        // Brightness of RGB LEDs is always 20% when in menu mode - 50/255
        wopr.defconLeds.setBrightness(50);
        RGB_Rainbow(10);

        // Timer to go into clock if no user interaction for XX seconds
        // If settings_clockCountdownTime is 0, this feature is off
        if (hasWiFi && config.clockCountdownTime > 0 && countdownToClock < millis()) {
            wopr.displayClear();
            currentMode = CLOCK;
            currentState = RUNNING;

            if (config.defconLedsBrightness == 0)
                wopr.defconLeds.clear();
        }
    }
        // We are running a simulation
    else {
        if (currentMode == CLOCK) {
            // If the Clock RGB brightness is not 0, show the rainbow at the current clock RGB brightness
            if (config.defconLedsBrightness > 0) {
                wopr.defconLeds.setBrightness(config.defconLedsBrightness);
                RGB_Rainbow(40);
            }

            if (nextBeep < millis()) {
                DisplayTime();
                nextBeep = millis() + 1000;
            }
        } else {
            // We have solved the code
            if (solveCount == solveCountFinished) {
                if (nextBeep < millis()) {
                    beeping = !beeping;
                    nextBeep = millis() + 500;

                    if (beeping) {
                        if (beepCount > 0) {
                            wopr.defconSetLevel(DefconLevel::DEFCON_1);
                            FillCodes();
                            beepCount--;
                            wopr.audioPlayTone(1500);
                        } else {
                            wopr.defconSetLevel(DefconLevel::DEFCON_1);
                            wopr.displaySetText("LAUNCHING...");
                        }
                    } else {
                        wopr.displayClear();
                        wopr.defconLeds.clear();
                        wopr.audioPlayTone(0);
                    }
                }

                // We are solved, so no point running any of the code below!
                return;
            }

            // Only update the displays every "tickStep"
            if (nextTick < millis()) {
                nextTick = millis() + tickStep;

                // This displays whatever the current state of the display is
                FillCodes();

                // If we are not currently beeping, play some random beep/bop computer-y sounds
                if (!beeping)
                    wopr.audioPlayTone(random(90, 250));
            }

            // This is where we solve each code digit
            // The next solve step is a random length to make it take a different time every run
            if (nextSolve < millis()) {
                nextSolve = millis() + solveStep;
                // Set the solve time step to a random length
                solveStep = GetNextSolveStep();
                //
                SolveCode();
            }

            // Zturn off any beeping if it's trying to beep
            if (beeping) {
                if (nextBeep < millis()) {
                    wopr.audioPlayTone(0);
                    beeping = false;
                }
            }
        }
    }
}

void loadSettings() {
    File settingsFile = LittleFS.open("/settings.json", "r");
    if (!settingsFile) {
        Serial.println("Failed to open settings file for reading!");
        return;
    }

    // Parse the JSON data from the file
    DynamicJsonDocument jsonDoc(256);  // Adjust the size according to your needs
    DeserializationError error = deserializeJson(jsonDoc, settingsFile);

    // Check for parsing errors
    if (error) {
        Serial.println("Error parsing settings JSON file!");
        return;
    }

    // Retrieve the values from the JSON document
    config.clockCountdownTime = jsonDoc["clockCountdownTime"];
    config.timeOffset = jsonDoc["timeOffset"];
    config.use24HourFormat = jsonDoc["use24HourFormat"];
    config.defconLedsBrightness = jsonDoc["defconLedsBrightness"];
    config.separatorCharacter = jsonDoc["separatorCharacter"];
    config.displayBrightness = jsonDoc["displayBrightness"];

    // Close the file
    settingsFile.close();
}


void saveSettings() {
    // Create a JSON document to hold the settings
    DynamicJsonDocument jsonDoc(256);  // Adjust the size according to your needs

    // Set the values in the JSON document
    jsonDoc["clockCountdownTime"] = config.clockCountdownTime;
    jsonDoc["timeOffset"] = config.timeOffset;
    jsonDoc["use24HourFormat"] = config.use24HourFormat;
    jsonDoc["defconLedsBrightness"] = config.defconLedsBrightness;
    jsonDoc["separatorCharacter"] = config.separatorCharacter;
    jsonDoc["displayBrightness"] = config.displayBrightness;

    // Open the settings file for writing
    File settingsFile = LittleFS.open("/settings.json", "w");
    if (!settingsFile) {
        Serial.println("Failed to open settings file for writing!");
        return;
    }

    // Serialize the JSON document to the file
    serializeJson(jsonDoc, settingsFile);

    // Close the file
    settingsFile.close();
}
