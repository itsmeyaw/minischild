# Product

<!-- impeccable:product-schema 1 -->

## Platform

android

## Users

Hobbyist owners who build and flash their own Minischild Device for personal use (home, desk). They open the Companion App rarely: to pair with the Device, set WiFi, and choose the Station, Route Directions and Platforms to show. The app is a configurator, not a daily-use surface; the Device is what they look at every day.

## Product Purpose

Minischild is a small always-on e-paper display (ESP32) that shows the upcoming public transport departures at one MVV station. The Companion App sets a Device's Board Selection and WiFi credentials from a phone over BLE. Success: a Device is paired and showing the right Departure Board in a few minutes, with no cloud account and no hand-edited `config.h` for day-to-day changes.

## Positioning

The phone does the Station, Route Direction and Platform lookup against the provider itself and sends only the chosen IDs to the Device over BLE. There is no cloud service, no account, and no search code or extra TLS traffic on the Device.

## Operating Context

- The Device advertises as `Minischild-XXXX` over BLE at all times.
- Pairing requires a 6-digit static passkey (`BLE_PASSKEY` from the Device's `config.h`) over an encrypted, authenticated link; up to 3 phones stay bonded.
- Settings are read and written as JSON on the `settings` characteristic (`docs/ble-protocol.md`). A valid write is saved, and the Device restarts only if the WiFi changed.
- The Device is typically within BLE range of the user's own phone, set up once and revisited when moving, changing commute, or changing WiFi.

## Capabilities and Constraints

- Flutter app in `app/`, single codebase (iOS and Android), BLE via `universal_ble` (BSD-3); `flutter_blue_plus` is excluded for licence reasons (ADR 0004).
- Terminology follows `GLOSSARY.md`: Station, Route, Route Direction, Headsign, Platform, Departure, Board Selection, Device, Companion App. Avoid "line", "stop", "config", "settings", "filter" for Board Selection.
- An empty list of Route Directions or Platforms means "show all".
- Station search and platform filtering are provider-driven (MVV first; see ADRs 0001, 0007), so the app depends on the provider's endpoints, which send no CORS headers.
- Personal-use scale; no multi-user, sharing or account features.

## Evidence on Hand

- ADRs in `docs/adr/` (0004 Flutter app, 0006 BLE passkey, 0007 platform filter) and the protocol spec in `docs/ble-protocol.md`.
- No user research, testimonials, or usage metrics exist; do not invent them.
- Only the Flutter scaffold exists in `app/lib/main.dart`; there is no established visual identity or screens yet.

## Product Principles

1. The Device is the product; the app is a tool that gets out of the way once it is configured.
2. Offline-simple: no accounts, no cloud, only what the phone and the Device need.
3. Use the Device's and the transit provider's own vocabulary (per `GLOSSARY.md`), not generic "settings" language.
4. Setup should fail legibly: pairing, passkey, WiFi and BLE errors say what to do next.
5. Android-native expectations (Material, permissions, back behavior) take precedence over cross-platform sameness.

## Accessibility & Inclusion

No product-specific requirement established. Follow Android accessibility defaults (TalkBack labels, touch targets, contrast, font scaling).
