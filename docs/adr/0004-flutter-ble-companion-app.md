# Companion App is a native Flutter app talking BLE

The Board Selection and WiFi credentials are set from a phone through a Flutter app (iOS first, Android too) that connects to the Device over BLE GATT using `universal_ble`. The app looks up Stations, Route Directions and Platforms from the provider itself and sends only the chosen IDs to the Device, which keeps the firmware free of search code and TLS traffic while BLE is busy.

## Considered Options

- **Settings web page served by the Device** (mDNS or SoftAP): no app or App Store, but WiFi onboarding needs a SoftAP detour, and the EFA station search sends no CORS headers, so the Device would have to proxy it.
- **Web Bluetooth**: iOS Safari doesn't support it.
- **Bluetooth Classic / SPP**: iOS only allows it for MFi accessories, so BLE is the only option.
- **`flutter_blue_plus`**: since v2 (Oct 2025) it is under the FlutterBluePlus License, which requires a paid licence for any for-profit use. `universal_ble` is BSD-3.
- **React Native, Kotlin Multiplatform, native Swift + Kotlin**: all feasible. Flutter was picked for one codebase with mature iOS BLE support, and it is already installed.
