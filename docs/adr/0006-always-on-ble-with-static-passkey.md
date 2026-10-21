# BLE is always on and protected by a static passkey from config.h

The Device advertises its BLE configuration service at all times, using NimBLE-Arduino. Its characteristics require an encrypted, authenticated link, paired with a 6-digit static passkey set as `BLE_PASSKEY` in `config.h`. Up to 3 bonded phones are kept. LE Secure Connections encrypts the link, so the WiFi password cannot be sniffed. The passkey is numeric only, because BLE pairing allows nothing else.

## Considered Options

- **Setup window** (BLE only for a few minutes after boot or a BOOT-button press): frees the BLE RAM afterwards and relies on physical access, but every change would need a trip to the Device.
- **Passkey shown on the e-paper**: needs a new `BoardView` screen for little gain over a fixed passkey on a personal Device.
- **Password checked by the app over an unencrypted link**: leaks the WiFi password to anyone listening.

## Consequences

Keeping NimBLE resident competes with TLS for heap on an ESP32 without PSRAM. The firmware logs its minimum free heap. If it drops below 30 KB after a full fetch with BLE running, revisit the setup-window option.
