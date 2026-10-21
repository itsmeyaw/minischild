#pragma once

#include <cstdint>
#include <string>
#include <vector>

// E-paper wiring for the Waveshare 2.9" V2 display on an ESP32.
// Copy this file to config.h, which is git-ignored.
constexpr int8_t PIN_BUSY = 4;
constexpr int8_t PIN_RST = 16;
constexpr int8_t PIN_DC = 17;
constexpr int8_t PIN_CS = 5;
constexpr int8_t PIN_CLK = 18;
constexpr int8_t PIN_DIN = 23;

// WiFi the board joins at boot.
constexpr const char* WIFI_SSID = "your-ssid";
constexpr const char* WIFI_PASSWORD = "your-password";

// 6-digit passkey a phone enters to pair with the Device over BLE (docs/ble-protocol.md).
// BLE pairing allows digits only. Change it: anyone in range who knows it can reconfigure the
// Device.
constexpr uint32_t BLE_PASSKEY = 123456;

// Station global ID and the Route Directions (MVV "stateless" IDs) to show. Find both via
// https://www.mvv-muenchen.de/?eID=departuresFinder&action=available_lines&stop_id=de%3A09162%3A2
// Leave ROUTE_DIRECTION_IDS empty ({}) to show all Routes at the Station.
// BVG uses an IBNR Station ID and "<line>|<IBNR>" Route Direction IDs (see provider/bvg/README.md).
// EFA providers use global IDs and "<line>|<destination ID>" (see provider/efa/README.md).
constexpr const char* STATION_ID = "de:09162:2";
inline const std::vector<std::string> ROUTE_DIRECTION_IDS = {"ddb:920S1: :H:j26"};
// Platforms to show, matched by exact text as the provider names them (e.g. "2", "U-Bahn Gleis 1").
// Leave empty ({}) to show every Platform; when set, Departures without a Platform are hidden.
inline const std::vector<std::string> PLATFORMS = {};
