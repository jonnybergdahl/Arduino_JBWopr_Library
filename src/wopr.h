//
// Created by Jonny Bergdahl on 2023-10-02.
//

#ifndef ARDUINO_WOPR_WOPR_H
#define ARDUINO_WOPR_WOPR_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_LEDBackpack.h>
#include <Adafruit_NeoPixel.h>
#include <OneButton.h>

enum WOPRBoardVersion {
    ORIGINAL = 0,
    HAXORZ
};

struct WOPRBoardPins {
    uint8_t buttonFrontLeftPin;
    uint8_t buttonFrontRightPin;
    uint8_t buttonBackTopPin;
    uint8_t buttonBackBottomPin;
    uint8_t defconLedsPin;
    uint8_t dacPin;
};

enum DefconLevel {
    DEFCON_1 = 0,
    DEFCON_2,
    DEFCON_3,
    DEFCON_4,
    DEFCON_5
};

class WOPRBoard {
public:
    WOPRBoard();

    void begin(WOPRBoardVersion version);
    void begin(WOPRBoardVersion version, WOPRBoardPins pins);
    void loop();

    WOPRBoardVersion wopr_version;

    // Display
    void displayClear();
    void displayShow();
    void displaySetBrightness(uint8_t val); // 0 - 15
    void displaySetText(String text);
    void displayScrollText(String text, uint16_t delay_ms = 100);

    void defconLedsSetColor(uint32_t color);
    void defconLedSetColor(DefconLevel level, uint32_t color);
    void defconSetLevel(DefconLevel level);

    void audioPlayTone(uint16_t freq);

    Adafruit_AlphaNum4 display[3] = { Adafruit_AlphaNum4(), Adafruit_AlphaNum4(), Adafruit_AlphaNum4() };

    // LED strip
    Adafruit_NeoPixel defconLeds = Adafruit_NeoPixel(5, 1, NEO_GRB + NEO_KHZ800);

    // Buttons
    OneButton buttonFrontLeft;
    OneButton buttonFrontRight;
    OneButton buttonBackTop;
    OneButton buttonBackBottom;

private:
    void _loadSettings();

    int _audioFreq = 2000;
    int _audioChannel = 0;
    int _audioResolution = 8;

    uint32_t _defcon_colors[5] = {
            Adafruit_NeoPixel::Color(255, 255, 255),
            Adafruit_NeoPixel::Color(255, 0, 0),
            Adafruit_NeoPixel::Color(255, 255, 0),
            Adafruit_NeoPixel::Color(0, 255, 0),
            Adafruit_NeoPixel::Color(0, 0, 255)
    };

};


#endif //ARDUINO_WOPR_WOPR_H
