# JBWOPR

JBWopr is a helper library for the Arduino platform that allows you to easily interface with the 
Unexpected Maker W.O.P.R. board.

## Installation

Install using the library manager in the Arduino IDE.

## Usage

### Basic JBWoprDevice usage

Create an instance of the `JBWoprDevice` class. Do any configuration you need to do then 
and call `begin()` in your `setup()` function. Then call `loop()` in your `loop()` function.

```cpp
#include <jbwopr.h>

JBWoprDevice wopr;

void setup() {
    // Initialize the JBWoprDevice class, passing in the board version 
    // JBWoprBoardVariant::HAXORZ or JBWoprBoardVariant::ORIGINAL
    wopr.begin(JBWoprBoardVariant::HAXORZ);
}

void loop() {
    wopr.loop();

    // Add your code here
}
```

The devices on the board are exposed via simple to use methods of the 
`JBWoprDevice` class.

```cpp
...
// Display
wopr.displaySetBrightness(50);
wopr.displaySetText("Hello World");
wopr.displaySetScrollText("Hello scrolling world");

// DEFCON
wopr.defconSetBrightness(50);
wopr.defconLedsSetDefconLebel(JBWoprDefconLevel::DEFCON1);

// Buttons
wopr.buttonFrontRightSetClickCallback(ButtonFrontRightClick);
...

void ButtonFrontRightClick() {
  Serial.println("Front right button clicked");
}
```

There is also support for running effects. There are some effects bundled with library,
but you can also create your own effects by inheriting from the `JBWoprEffectBase` class.

```cpp
#include <jbwopr.h>
#include <effects/jbwopreffets.h>

...


```

### Advanced JBWoprDevice usage

The class exposes the onboard devices as the following:

* The 12 character display are exposed as an array of `Adafruit_AlphaNum4`devices
* The 5 DEFCON LED's are exposed as a `Adafruit_NeoPixel` device
* The  buttons are exposed as `OneButton` devices

```cpp
#include <jbwopr.h>

JBWoprDevice wopr;

void setup() {
    // Initialize the JBWoprDevice class, passing in the board version 
    // JBWoprBoardVariant::HAXORZ or JBWoprBoardVariant::ORIGINAL
    wopr.begin(JBWoprBoardVariant::HAXORZ);
}

void loop() {
    wopr.loop();

    // Add your code here
}
```



The library sets the device id to `wopr-<mac_address>`. You can change this by calling `wopr.setDeviceId("my-device-id")` before calling `wopr.begin()`.

The `WOPR` class exposes the defcon LED's as a single LED strip via `wopr.defconLeds`. It uses the Adafruit_NeoPixel library.

# MQTT

## MQTT Topics

Device configuration is posted to the `<mqtt prefix>/<device_id>/config` topic. It is posted at startup and when changes are made by a command.
Device state is posted to the `<mqtt prefix>/<device_id>/state` topic. It is posted at startup and when state is changed by a command.

### Publishing state from the W.O.P.R.

| Topic                        | Payload | Comment   |
|------------------------------|---------|-----------|
| wopr/wopr-jonny/availability | ONLINE | HA only   |

| Topic                              | Payload               | Comment |
|------------------------------------|-----------------------|---------|
| wopr/wopr-jonny/display/state      | ON / OFF              |         |
| wopr/wopr-jonny/display/text       | Hello World           |         |
| wopr/wopr-jonny/display/scrolltext | Hello scrolling world |         |
| wopr/wopr-jonny/display/brightness | 50                    |         |

### Settings state from another device: Commands

wopr/wopr-jonny/display/text/set
wopr/wopr-jonny/display/brightness/set

## Home Assistant
The state payload is a JSON object with the following properties:

```json
{
  "display": {
    "state": "ON",
    "brightness": 50,
    "text": "Hello World"
  },
  "defcon": {
    "state": "ON",
    "brightness": 75,
    "color": "255,0,0",
  },
  "effect": {
    "state": "ON",
    "name": "Rainbow"
  },
  "diagnostics": {
    "ipAddress": "172.30.4.5",
    "rssi": -50,
    "version": "1.0.0"
  }
}
``` 

The configuration payload is a JSON object with the following properties:
```json
{
  "effectsTimeout": 30,
  "separatorCharacter": 0,
  "timeGmtOffset": 2,
  "use24HourFormat": true,
  "defconLedsBrightness": 128,
  "displayBrightness": 128,
  "useWebPortal": true,
  "useMqtt": true,
  "mqttServerName": "192.168.1.100",
  "mqttServerPort": 1080,
  "mqttUserName": "user",
  "mqttPassword": "password",
  "useHomeAssistant": true,
  "homeAssistantDiscoveryPrefix": "homeassistant"
}
```
