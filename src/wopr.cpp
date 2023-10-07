//
// Created by Jonny Bergdahl on 2023-10-02.
//

#include "wopr.h"
#include <esp_system.h>

WOPRBoard::WOPRBoard() {
}

void WOPRBoard::begin(WOPRBoardVersion version) {
#if defined(ESP32S2)
    WOPRBoardPins pinAssignments = {
        .buttonFrontLeftPin = 5,
        .buttonFrontRightPin = 6,
        .buttonBackTopPin = 38,
        .buttonBackBottomPin = 33,
        .defconLedsPin = 7,
        .dacPin = 18
};
#elif defined(ESP32S3)
    WOPRBoardPins pinAssignments = {
        .buttonFrontLeftPin = 2,
        .buttonFrontRightPin = 3,
        .buttonBackTopPin = 7,
        .buttonBackBottomPin = 6,
        .defconLedsPin = 4,
        .dacPin = 21
};
#else
    WOPRBoardPins pinAssignments = {
            .buttonFrontLeftPin = 15,
            .buttonFrontRightPin = 14,
            .buttonBackTopPin = 32,
            .buttonBackBottomPin = 33,
            .defconLedsPin = 27,
            .dacPin = 25

    };
#endif
    begin(version, pinAssignments);
}

void WOPRBoard::begin(WOPRBoardVersion version, WOPRBoardPins pins) {
    wopr_version = version;

    buttonFrontLeft = OneButton(pins.buttonFrontLeftPin, false);
    buttonFrontRight = OneButton(pins.buttonFrontRightPin, false);
    if (wopr_version == WOPRBoardVersion::HAXORZ) {
        buttonBackTop = OneButton(pins.buttonBackTopPin, false);
        buttonBackBottom = OneButton(pins.buttonBackBottomPin, false);
    }

    // Address  0x70 == 1, 0x72 == 2, 0x74 == 3
    display[0].begin(0x70);
    display[1].begin(0x72);
    display[2].begin(0x74);
    displayClear();

    // Initialise the NeoPixel strip
    defconLeds.setPin(pins.defconLedsPin);
    defconLeds.begin();
    defconLeds.clear();
    defconLeds.show();

    // Audio
    Serial.println("WOPRBoard define audio");
    if (ledcSetup(_audioChannel, _audioFreq, _audioResolution) == 0)
    {
        Serial.println("FAIL!");
        return;
    };
    ledcAttachPin(pins.dacPin, _audioChannel);

    Serial.println("WOPRBoard done");
}

void WOPRBoard::loop()
{
    // Used by OneButton to poll for button inputs
    buttonFrontLeft.tick();
    buttonFrontRight.tick();
    if (wopr_version == WOPRBoardVersion::HAXORZ) {
        buttonBackTop.tick();
        buttonBackBottom.tick();
    }
}

void WOPRBoard::displaySetText(String txt)
{
    uint8_t curDisplay = 0;
    uint8_t curDigit = 0;

    displayClear();

    // Iterate through each digit and push the character rom the txt string into that position
    for ( uint8_t i = 0; i < txt.length(); i++ )
    {
        display[curDisplay].writeDigitAscii( curDigit, txt.charAt(i));
        curDigit++;
        if (curDigit == 4)
        {
            curDigit = 0;
            curDisplay++;
        }
    }
    displayShow();
}

void WOPRBoard::displayScrollText(String text, uint16_t delay_ms) {
    uint8_t curDisplay = 0;
    uint8_t curDigit = 0;

    displayClear();

    String padded_text = "            " + text + "            ";
    int text_len = padded_text.length();
    int start_index = 0;

    // Iterate through each digit and push the character from the txt string into that position
    for (uint8_t i = 0; i < padded_text.length() - 12; i++) {
        for (uint8_t d = 0; d < 12; d++) {
            display[curDisplay].writeDigitAscii(curDigit, padded_text.charAt(start_index + d));
            curDigit++;
            if (curDigit == 4) {
                curDigit = 0;
                curDisplay++;
            }
        }

        displayShow();
        delay(delay_ms);
        start_index++;

        curDisplay = 0;
        curDigit = 0;
    }
}

void WOPRBoard::displayClear()
{
    for ( int i = 0; i < 3; i++ ) {
        for (int d = 0; d < 4; d++) {
            display[i].writeDigitAscii(d, ' ');
        }
    }
    displayShow();
}

void WOPRBoard::displayShow()
{
    // display.show() ??
    display[0].writeDisplay();
    display[1].writeDisplay();
    display[2].writeDisplay();
}

void WOPRBoard::displaySetBrightness(uint8_t value)
{
    value = constrain(value, 0, 15);
    for (int x = 0; x < 3; x++) {
        display[x].setBrightness(value);
    }
}

void WOPRBoard::defconLedsSetColor(uint32_t color)
{
    for (int i = 0; i < 5; i++)
        defconLeds.setPixelColor(i, color);
    defconLeds.show();
}

void WOPRBoard::defconLedSetColor(DefconLevel level, uint32_t color)
{
    uint32_t led = 4 - constrain(level, 0, 5);
    Serial.printf("level = %i, led = %i\n", level, led);
    defconLeds.setPixelColor(led, color);
    defconLeds.show();
}

void WOPRBoard::defconSetLevel(DefconLevel level)
{
    defconLeds.clear();
    defconLedSetColor(level, _defcon_colors[level]);
    defconLeds.show();
}

void WOPRBoard::audioPlayTone(uint16_t freq)
{
    ledcWriteTone(_audioChannel, freq);
}





