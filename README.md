<div align="center">

# Minischild

A small always-on display that shows the upcoming public transport departures at one station.

</div>

Minischild runs on an ESP32 and renders a live departure board on an e-paper display. It fetches realtime data from a transit provider (MVV out of the box), shapes it into a GTFS Realtime-like model, and lets any display implementation render it — no code changes needed to swap hardware or data source.

## Requirements

- [PlatformIO](https://platformio.org/) (CLI or the VS Code extension)
- An ESP32 board
- A supported e-paper display (ships with a Waveshare 2.9" V2 driver via [GxEPD2](https://github.com/ZinggJM/GxEPD2))
- A WiFi network with internet access, for fetching departures and syncing the clock over NTP

> [!NOTE]
> The native test suite (`pio test -e native`) builds and runs without any hardware.

## Setup

1. Copy `include/config.example.h` to `include/config.h`, then fill in:
   - WiFi SSID and password
   - the Station ID and, optionally, the Route Direction IDs to show (leave the list empty to show every Route at the Station)
   - the e-paper pin wiring, if it differs from the default
2. Flash the board:

   ```sh
   pio run -e esp32dev -t upload
   ```

3. Run the native tests:

   ```sh
   pio test -e native
   ```

### Picking the view and provider

The build environment picks the board; the view (display) and provider (data source) are picked independently, defaulting to `custom_view` / `custom_provider` in `platformio.ini` (Waveshare 2.9" e-paper + MVV):

```sh
pio run -e esp32dev -t upload                # epaper_290 + mvv
VIEW=epaper_42 pio run -e esp32dev -t upload # another view, same provider
VIEW=epaper_42 PROVIDER=gtfs pio run -e esp32dev   # swap both
```

A name is a folder under `src/view/` or `src/provider/`; a name with no matching folder stops the build and lists the available ones. Put any settings the chosen view or provider needs in `config.h`.

## Extending

> [!IMPORTANT]
> Add new views and providers next to the existing ones; don't edit one that already works.

### Add a display

1. Implement `BoardView` (`include/view/board_view.h`) in `include/view/<name>/` and `src/view/<name>/`.
2. In `src/view/<name>/setup.cpp`, define `BoardView& setupBoardView()` (`include/setup.h`): initialise the hardware and return the view.
3. Put any pins or settings the view needs in `config.example.h`.
4. Build with `VIEW=<name> pio run -e esp32dev`.

### Add a data provider

Same pattern: implement `DepartureProvider` (`include/provider/departure_provider.h`) in `include/provider/<name>/` and `src/provider/<name>/`, define `DepartureProvider& setupDepartureProvider()` in `src/provider/<name>/setup.cpp`, and build with `PROVIDER=<name> pio run -e esp32dev`. Map the source into the GTFS Realtime-shaped model (`docs/adr/0002-gtfs-realtime-shaped-model.md`) so every view can render it.

A folder without its `setup.cpp` fails to link with `undefined reference to setupBoardView()` (or `setupDepartureProvider()`).

## Project layout

```
include/, src/         board, model, provider, util and view code
src/view/<name>/       one BoardView implementation per supported display
src/provider/<name>/   one DepartureProvider implementation per data source
test/                  native Unity tests, one folder per unit under test
docs/adr/              architecture decisions
```

## Further reading

- [`GLOSSARY.md`](GLOSSARY.md) — domain language used throughout the codebaseA
- Each provider's `README.md` inside `include/providers/`
- [`CODING_STANDARDS.md`](CODING_STANDARDS.md) — principles the code follows
- [`docs/adr/`](docs/adr) — why the view/provider split and the GTFS-shaped model exist
