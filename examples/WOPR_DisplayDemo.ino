#include <wopr.h>

WOPRBoard wopr;

void setup() {
    // wopr.begin(WOPRBoardVersion::ORIGINAL);
    wopr.begin(WOPRBoardVersion::HAXORZ);
}

void loop() {
    wopr.displaySetText("WOPR Demo");
    delay(2000);
    wopr.displaySetText("BRIGHTNESS");
    for (uint8_t i = 15; i > 0; i--)
    {
        wopr.displaySetBrightness(i);
        delay(150);
    }
    for (uint8_t i = 0; i <= 15; i++)
    {
        wopr.displaySetBrightness(i);
        delay(150);
    }
    wopr.displayScrollText("SCROLLING TEXT AT DEFAULT RATE");
    wopr.displayScrollText("SCROLLING TEXT 200 MILLIS RATE - ABCDEFGHIJKLMNOPQRSTVWXYZ abscdefghijklmnopqrstvwxyz 0123456789", 200);
    delay(1000);
}
