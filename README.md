# Minischild

A stop schedule viewer fore ESP32 board.

## Build

1. Copy `include/config.example.h` to `include/config.h`, then set the WiFi, Station and Route Directions, and adjust the pins if needed.
2. Flash the ESP32: `pio run -e esp32dev -t upload`
3. Run the native tests: `pio test -e native`
