#pragma once

#include <cstdint>

// E-paper wiring for the Waveshare 2.9" V2 display on an ESP32.
// Copy this file to config.h, which is git-ignored.
constexpr int8_t PIN_BUSY = 4;
constexpr int8_t PIN_RST = 16;
constexpr int8_t PIN_DC = 17;
constexpr int8_t PIN_CS = 5;
constexpr int8_t PIN_CLK = 18;
constexpr int8_t PIN_DIN = 23;
