# Arduino_WOPR

Arduino_WOPR is a helper library for the Arduino platform that allows you to easily interface with the Unexpected Maker W.O.P.R. board.

## Installation

Install using the library manager in the Arduino IDE.

## Usage

```cpp
#include <Arduino_WOPR.h>

WOPR wopr;

void setup() {
    // Initalize the WOPR class, passing in the board version 
    // WOPRBoardVersion::HAXORZ or WOPRBoardVersion::ORIGINAL
    wopr.begin(WOPRBoardVersion::HAXORZ);
}

void loop() {
    wopr.loop();

    // Add your code here
}
```

The `WOPR` class exposes the defcon LED's as a single LED strip via `wopr.defconLeds`. It uses the Adafruit_NeoPixel library.

