#pragma once

#include <ctime>
#include <string>

#include "departure_board.h"

inline time_t departureTime(const Departure& d) {
  return d.departure ? d.departure->time : d.scheduledDepartureTime;
}

inline std::string countdownLabel(const Departure& d, time_t now) {
  const time_t minutes = (departureTime(d) - (now - now % 60)) / 60;
  if (minutes <= 0) return "now";
  return std::to_string(static_cast<long long>(minutes)) + " min";
}
