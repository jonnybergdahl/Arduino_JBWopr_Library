#include <wopr.h>

WOPRBoard wopr;

void setup() {
    // wopr.begin(WOPRBoardVersion::ORIGINAL);
    wopr.begin(WOPRBoardVersion::HAXORZ);
}

void loop() {
    wopr.defconSetLevel(DefconLevel::DEFCON_5);
    delay(2000);
    wopr.defconSetLevel(DefconLevel::DEFCON_4);
    delay(2000);
    wopr.defconSetLevel(DefconLevel::DEFCON_3);
    delay(2000);
    wopr.defconSetLevel(DefconLevel::DEFCON_2);
    delay(2000);
    wopr.defconSetLevel(DefconLevel::DEFCON_1);
    delay(2000);
}
