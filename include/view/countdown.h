#pragma once

#include <ctime>
#include <string>

#include "model/departure_board.h"

inline std::string countdownLabel(const Departure& d, time_t now) {
  const time_t minutes = (departureTime(d) - (now - now % 60)) / 60;
  if (minutes <= 0) return "now";
  return std::to_string(static_cast<long long>(minutes)) + " min";
}
