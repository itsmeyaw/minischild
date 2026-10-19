# Minischild

A stop schedule viewer fore ESP32 board.

## Build

1. Copy `include/config.example.h` to `include/config.h`, then set the WiFi, Station and Route Directions, and adjust the pins if needed.
2. Flash the ESP32: `pio run -e esp32dev -t upload`
3. Run the native tests: `pio test -e native`

The `esp32dev` env builds the Waveshare 2.9" e-paper view with the MVV provider.

## Add a display

Add a new view next to the existing one; don't edit the one that works.

1. Implement `BoardView` (`include/view/board_view.h`) in `include/view/<name>/` and `src/view/<name>/`.
2. In `src/view/<name>/setup.cpp`, define `BoardView& setupBoardView()` (`include/setup.h`): initialise the hardware and return the view.
3. Add an env to `platformio.ini`:

   ```ini
   [env:<device>]
   extends = esp32_base
   build_src_filter = +<main.cpp> +<view/<name>/> +<provider/mvv/>
   ```

4. Build with `pio run -e <device>`. Put any pins or settings the view needs in `config.example.h`. Swap `provider/mvv/` for another provider the same way (below).

## Add a data provider

Same pattern: implement `DepartureProvider` (`include/provider/departure_provider.h`) in `provider/<name>/`, define `DepartureProvider& setupDepartureProvider()` in `src/provider/<name>/setup.cpp`, and pick `+<provider/<name>/>` in your env's `build_src_filter`. Map the source into the GTFS Realtime-shaped model (`docs/adr/0002-gtfs-realtime-shaped-model.md`) so every view can render it.

Each env must pick exactly one view and one provider. If it picks none, the link fails with `undefined reference to setupBoardView()`; if it picks two, it fails with `multiple definition`.
