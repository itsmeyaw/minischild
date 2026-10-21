# Firmware and Companion App share one repository

The PlatformIO firmware lives in `firmware/` and the Flutter Companion App in `app/`. They share `docs/` (ADRs and `docs/ble-protocol.md`, the GATT contract both sides implement) and one root `GLOSSARY.md`, because both speak the same domain language. Keeping them together lets a change to the BLE contract land on both sides in one stack of PRs. Each toolchain stays inside its own folder, so CI runs per folder.
