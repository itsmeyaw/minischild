#pragma once

#include <algorithm>
#include <cstddef>
#include <ctime>
#include <vector>

#include "model/departure_board.h"

constexpr size_t kBoardSize = 5;

// Upcoming Departures soonest first, at most kBoardSize. A Departure in the current minute still
// counts as upcoming, matching countdownLabel's "now".
inline std::vector<Departure> selectDepartures(std::vector<Departure> departures, time_t now) {
  const time_t currentMinute = now - now % 60;
  departures.erase(
      std::remove_if(departures.begin(), departures.end(),
                     [&](const Departure& d) { return departureTime(d) < currentMinute; }),
      departures.end());
  std::stable_sort(
      departures.begin(), departures.end(),
      [](const Departure& a, const Departure& b) { return departureTime(a) < departureTime(b); });
  departures.resize(std::min(departures.size(), kBoardSize));
  return departures;
}
