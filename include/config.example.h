#pragma once

#include <cstdint>
#include <initializer_list>

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

// Station global ID and the Route Directions (MVV "stateless" IDs) to show. Find both via
// https://www.mvv-muenchen.de/?eID=departuresFinder&action=available_lines&stop_id=de%3A09162%3A2
// Leave ROUTE_DIRECTION_IDS empty ({}) to show all Routes at the Station.
constexpr const char* STATION_ID = "de:09162:2";
constexpr std::initializer_list<const char*> ROUTE_DIRECTION_IDS = {"ddb:920S1: :H:j26"};
