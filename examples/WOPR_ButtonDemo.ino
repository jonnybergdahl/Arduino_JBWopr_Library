#include <wopr.h>

WOPRBoard wopr;

void setup() {
    Serial.begin(9600);

    // wopr.begin(WOPRBoardVersion::ORIGINAL);
    wopr.begin(WOPRBoardVersion::HAXORZ);

    Serial.begin(115200);

    // Attach button IO for OneButton
    wopr.buttonFrontRight.attachClick(ButtonFrontRightPress);
    wopr.buttonFrontRight.attachLongPressStop(ButtonFrontRightLongPress);
    wopr.buttonFrontRight.attachDoubleClick(ButtonFrontRightDoubleClick);
    wopr.buttonFrontLeft.attachClick(ButtonFrontLeftPress);
    wopr.buttonFrontLeft.attachLongPressStop(ButtonFrontLeftLongPress);
    wopr.buttonFrontLeft.attachDoubleClick(ButtonFrontLeftDoubleClick);
    if (wopr.wopr_version == WOPRBoardVersion::HAXORZ) {
        wopr.buttonBackTop.attachClick(ButtonBackTopPress);
        wopr.buttonBackBottom.attachClick(ButtonBackBottomPress);
    }
}

void loop() {
    wopr.loop();
}

void ButtonFrontRightPress() {
    Serial.println("Front right button pressed");
}

void ButtonFrontRightDoubleClick() {
    Serial.println("Front right button double clicked");
}

void ButtonFrontRightLongPress() {
    Serial.println("Front right button long pressed");
}

void ButtonFrontLeftPress() {
    Serial.println("Front left button pressed");
}

void ButtonFrontLeftDoubleClick() {
    Serial.println("Front left button double clicked");
}

void ButtonFrontLeftLongPress() {
    Serial.println("Front left button long pressed");
}

void ButtonBackTopPress() {
    Serial.println("Back top button pressed");
}

void ButtonBackBottomPress() {
    Serial.println("Back bottom button pressed");
}