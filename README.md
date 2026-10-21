<div align="center">

# Minischild

A small always-on display that shows the upcoming public transport departures at one station.

</div>

Minischild runs on an ESP32 and renders a live departure board on an e-paper display. It fetches realtime data from a transit provider (MVV, BVG, and the EFA regions VVS, VRR, KVV and Bavaria out of the box), shapes it into a GTFS Realtime-like model, and lets any display implementation render it.

## Requirements

- [PlatformIO](https://platformio.org/) (CLI or the VS Code extension)
- An ESP32 board
- A supported e-paper display (ships with a Waveshare 2.9" V2 driver via [GxEPD2](https://github.com/ZinggJM/GxEPD2))
- A WiFi network with internet access, for fetching departures and syncing the clock over NTP

> [!NOTE]
> The native test suite (`pio test -e native`) builds and runs without any hardware.

## Setup

The firmware is a PlatformIO project in `firmware/`; run every `pio` command below from there.

1. Copy `firmware/include/config.example.h` to `firmware/include/config.h`, then fill in:
   - WiFi SSID and password
   - the BLE passkey, which you enter when pairing a phone (see [Changing settings over BLE](#changing-settings-over-ble))
   - the Station ID and, optionally, the Route Direction IDs and Platforms to show (leave a list empty to show everything of that kind)
   - the e-paper pin wiring, if it differs from the default

   Settings written over BLE (see [`docs/ble-protocol.md`](docs/ble-protocol.md)) are stored on the Device and override the WiFi and Board Selection values in `config.h`.
2. Flash the board:

   ```sh
   pio run -e esp32dev -t upload
   ```

3. Run the native tests:

   ```sh
   pio test -e native
   ```

### Picking the view and provider

The build environment picks the board; the view (display) and provider (data source) are picked independently, defaulting to `custom_view` / `custom_provider` in `firmware/platformio.ini` (Waveshare 2.9" e-paper + MVV):

```sh
pio run -e esp32dev -t upload                # epaper_290 + mvv
VIEW=epaper_42 pio run -e esp32dev -t upload # another view, same provider
PROVIDER=bvg pio run -e esp32dev -t upload   # epaper_290 + bvg
PROVIDER=vvs pio run -e esp32dev -t upload   # epaper_290 + vvs (Stuttgart, an EFA region)
VIEW=epaper_42 PROVIDER=gtfs pio run -e esp32dev   # swap both
```

A name is a folder under `firmware/src/view/` or `firmware/src/provider/`; a name with no matching folder stops the build and lists the available ones. Put any settings the chosen view or provider needs in `config.h`. The `vvs`, `vrr`, `kvv` and `bavaria` providers share one EFA core and differ only in their region's base URL (see [`firmware/include/provider/efa/README.md`](firmware/include/provider/efa/README.md)).

### Changing settings over BLE

The Device advertises as `Minischild-XXXX` at all times. Pair with a generic BLE tool such as nRF Connect, entering `BLE_PASSKEY` from `config.h`. Read and write the `settings` characteristic as JSON as described in [`docs/ble-protocol.md`](docs/ble-protocol.md). A valid write is saved and the Device restarts. The serial log prints the minimum free heap whenever it reaches a new low.

## Extending

> [!IMPORTANT]
> Add new views and providers next to the existing ones; don't edit one that already works.

Paths below are relative to `firmware/`.

### Add a display

1. Implement `BoardView` (`include/view/board_view.h`) in `include/view/<name>/` and `src/view/<name>/`.
2. In `src/view/<name>/setup.cpp`, define `BoardView& setupBoardView()` (`include/setup.h`): initialise the hardware and return the view.
3. Put any pins or settings the view needs in `config.example.h`.
4. Build with `VIEW=<name> pio run -e esp32dev`.

### Add a data provider

Same pattern: implement `DepartureProvider` (`include/provider/departure_provider.h`) in `include/provider/<name>/` and `src/provider/<name>/`, define `DepartureProvider& setupDepartureProvider(const BoardSelection&)` in `src/provider/<name>/setup.cpp`, taking the Station and Route Directions from the Board Selection rather than `config.h`, and build with `PROVIDER=<name> pio run -e esp32dev`. Map the source into the GTFS Realtime-shaped model (`docs/adr/0002-gtfs-realtime-shaped-model.md`) so every view can render it.

A folder without its `setup.cpp` fails to link with `undefined reference to setupBoardView()` (or `setupDepartureProvider()`).

## Project layout

```
firmware/                         PlatformIO project for the Device
firmware/include/, firmware/src/  board, model, provider, util and view code
firmware/src/view/<name>/         one BoardView implementation per supported display
firmware/src/provider/<name>/     one DepartureProvider implementation per data source (mvv, bvg, vvs, vrr, kvv, bavaria)
firmware/test/                    native Unity tests, one folder per unit under test
firmware/scripts/                 build scripts (view and provider selection)
app/                              Flutter Companion App (see app/README.md)
docs/adr/                         architecture decisions, shared by every folder
GLOSSARY.md                       domain language, shared by every folder
```

## Further reading

- [`GLOSSARY.md`](GLOSSARY.md) — domain language used throughout the codebase
- [`docs/ble-protocol.md`](docs/ble-protocol.md) — the BLE contract between the Device and the Companion App
- [`app/README.md`](app/README.md) — the Companion App: running, checks and sideloading
- Each provider's `README.md` inside [`firmware/include/provider/`](firmware/include/provider) (`mvv/`, `bvg/`, `efa/`, ...)
- [`CODING_STANDARDS.md`](CODING_STANDARDS.md) — principles the code follows
- [`docs/adr/`](docs/adr) — why the view/provider split and the GTFS-shaped model exist
