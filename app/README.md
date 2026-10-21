# Minischild Companion App

The **Companion App** is the phone app that sets a Device's Board Selection over BLE. This folder holds its Flutter project; firmware lives in [`../firmware/`](../firmware/README.md) and shared decisions in [`../docs/adr/`](../docs/adr) (see [ADR 0004](../docs/adr/0004-flutter-ble-companion-app.md)).

App ID: `id.itsmeyaw.minischild`

## Prerequisites

- [Flutter](https://docs.flutter.dev/get-started/install) stable (3.35.7 is what CI pins)
- For iOS: Xcode, with an iPhone running a recent iOS
- For Android: the Android SDK (`flutter doctor` lists what's missing)

## Run

Run every command from this folder.

```sh
flutter pub get
flutter run   # with a phone attached
```

## Checks

```sh
flutter analyze
flutter test
```

## Sideload on iOS

1. Open `ios/Runner.xcworkspace` in Xcode.
2. Select the Runner target, then **Signing & Capabilities**, and pick a **Team**. A free Apple ID works, but free provisioning expires after 7 days, so the app must be re-installed then.
3. Enable **Developer Mode** on the iPhone (Settings → Privacy & Security → Developer Mode).
4. Install the release build:

   ```sh
   flutter run --release
   ```

5. On first launch, trust the developer under Settings → General → VPN & Device Management.

## Sideload on Android

```sh
flutter build apk --release
adb install build/app/outputs/flutter-apk/app-release.apk
```

Without `adb`, copy `build/app/outputs/flutter-apk/app-release.apk` to the phone and allow installing unknown apps for the app that opens it.

The release APK is signed with the local debug key, which is fine for sideloading but not for a store.

## Replace the icon

The launcher icon is generated from `assets/icon/icon.png`. Replace that file with a 1024×1024 PNG, then regenerate the platform icons:

```sh
dart run flutter_launcher_icons
```
