# Minischild

A stop schedule viewer fore ESP32 board.

## Build

1. Copy `include/config.example.h` to `include/config.h`, then set the WiFi, Station and Route Directions (leave the list empty for all Routes at the Station), and adjust the pins if needed.
2. Flash the ESP32: `pio run -e esp32dev -t upload`
3. Run the native tests: `pio test -e native`

## Pick the view and provider

The env picks the board; the view and provider are picked independently, defaulting to `custom_view` / `custom_provider` in `platformio.ini` (Waveshare 2.9" e-paper + MVV):

```sh
pio run -e esp32dev -t upload                           # epaper_290 + mvv
VIEW=epaper_42 pio run -e esp32dev -t upload            # another view, same provider
VIEW=epaper_42 PROVIDER=gtfs pio run -e esp32dev        # swap both
```

A name is a folder under `src/view/` or `src/provider/`; a name with no folder stops the build and lists the available ones. Put the settings the chosen view and provider need in `config.h`.

## Add a display

Add a new view next to the existing one; don't edit the one that works.

1. Implement `BoardView` (`include/view/board_view.h`) in `include/view/<name>/` and `src/view/<name>/`.
2. In `src/view/<name>/setup.cpp`, define `BoardView& setupBoardView()` (`include/setup.h`): initialise the hardware and return the view.
3. Put any pins or settings the view needs in `config.example.h`.
4. Build with `VIEW=<name> pio run -e esp32dev`.

## Add a data provider

Same pattern: implement `DepartureProvider` (`include/provider/departure_provider.h`) in `include/provider/<name>/` and `src/provider/<name>/`, define `DepartureProvider& setupDepartureProvider()` in `src/provider/<name>/setup.cpp`, and build with `PROVIDER=<name> pio run -e esp32dev`. Map the source into the GTFS Realtime-shaped model (`docs/adr/0002-gtfs-realtime-shaped-model.md`) so every view can render it.

A folder without its `setup.cpp` fails to link with `undefined reference to setupBoardView()` (or `setupDepartureProvider()`).
