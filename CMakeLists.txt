cmake_minimum_required(VERSION 3.26)
project(Arduino_WOPR)

set(CMAKE_CXX_STANDARD 14)

include_directories(src)
include_directories("~/Documents/Arduino/libraries/Adafruit_NeoPixel/src")
include_directories("~/Documents/Arduino/libraries/Adafruit_LED_Backpack/src")
include_directories("~/Documents/Arduino/libraries/ArduinoJson/src")
include_directories("~/Documents/Arduino/libraries/WiFiManager")
include_directories("~/Documents/Arduino/libraries/PubSubClient/src")
include_directories("~/Documents/Arduino/libraries/JBLogger/src")
include_directories("~/Library/Arduino15/packages/esp32/hardware/esp32/2.0.14/cores/esp32")
include_directories("~/Library/Arduino15/packages/esp32/hardware/esp32/2.0.14/libraries")
include_directories("~/Library/Arduino15/packages/esp32/hardware/esp32/2.0.14/tools/sdk/esp32/include")
include_directories("~/Library/Arduino15/packages/esp32/hardware/esp32/2.0.14/tools/sdk/esp32/include/freertos/include")
include_directories("~/Library/Arduino15/packages/esp32/hardware/esp32/2.0.14/tools/sdk/esp32/include/freertos/include/esp_additions/freertos")
include_directories("~/Library/Arduino15/packages/esp32/hardware/esp32/2.0.14/tools/sdk/esp32/dout_qspi/include/")
include_directories("~/Library/Arduino15/packages/esp32/hardware/esp32/2.0.14/tools/sdk/esp32/include/esp_common/include/")
include_directories("~/Library/Arduino15/packages/esp32/hardware/esp32/2.0.14/tools/sdk/esp32/include/freertos/port/xtensa/include/")
include_directories("~/Library/Arduino15/packages/esp32/hardware/esp32/2.0.14/tools/sdk/esp32/include/xtensa/include/")
include_directories("~/Library/Arduino15/packages/esp32/hardware/esp32/2.0.14/tools/sdk/esp32/include/xtensa/esp32/include/")
include_directories("~/Library/Arduino15/packages/esp32/hardware/esp32/2.0.14/tools/sdk/esp32/include/esp_rom/include/")
include_directories("~/Library/Arduino15/packages/esp32/hardware/esp32/2.0.14/tools/sdk/esp32/include/esp_rom/include/linux/")
include_directories("~/Library/Arduino15/packages/esp32/hardware/esp32/2.0.14/tools/sdk/esp32/include/esp_hw_support/include/")
include_directories("~/Library/Arduino15/packages/esp32/hardware/esp32/2.0.14/tools/sdk/esp32/include/hal/include/")
include_directories("~/Library/Arduino15/packages/esp32/hardware/esp32/2.0.14/tools/sdk/esp32/include/soc/esp32/include/")
include_directories("~/Library/Arduino15/packages/esp32/hardware/esp32/2.0.14/tools/sdk/esp32/include/hal/esp32/include/")

add_executable(Arduino_WOPR
        src/ha/mdi_consts.h
        src/jbwopr.cpp
        src/jbwopr.h
        src/jbwoprwifi.h
        src/jbwoprwifi.cpp
        src/jbwoprmqtt.h
        src/jbwoprmqtt.cpp
        src/jbwoprha.h
        src/jbwoprha.cpp
        src/jbwoprhelpers.h
        src/jbwoprhelpers.cpp
        src/effects/jbwopreffects.h
        src/effects/jbwopreffects.cpp
        src/effects/jbwoprtherickeffect.h
        src/effects/jbwoprtherickeffect.cpp
        src/effects/jbwoprnokiatune.h
        src/effects/jbwoprnokiatune.cpp
        examples/JBWopr_AudioDemo/JBWopr_AudioDemo.ino
        examples/JBWopr_ButtonDemo/JBWopr_ButtonDemo.ino
        examples/JBWopr_DeviceDemo/JBWopr_DeviceDemo.ino
        examples/JBWopr_DefconDemo/JBWopr_DefconDemo.ino
        examples/JBWopr_DisplayDemo/JBWopr_DisplayDemo.ino
        examples/JBWopr_Firmware/JBWopr_Firmware.ino

)

