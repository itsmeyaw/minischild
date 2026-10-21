#pragma once

#include <algorithm>
#include <cstddef>
#include <ctime>
#include <string>
#include <vector>

#include "model/departure_board.h"

// The Station, Route Directions and Platforms shown on a Device; an empty list means all of them.
struct BoardSelection {
  std::string stationId;
  std::vector<std::string> routeDirectionIds;
  std::vector<std::string> platforms;
};

// Upcoming Departures soonest first, at most limit. A Departure in the current minute still
// counts as upcoming, matching countdownLabel's "now". With platforms listed, only Departures
// whose Platform matches one of them exactly are kept; an empty list keeps every Platform.
inline std::vector<Departure> selectDepartures(const std::vector<Departure>& departures,
                                               const std::vector<std::string>& platforms,
                                               time_t now, size_t limit) {
  const time_t currentMinute = now - now % 60;
  std::vector<const Departure*> upcoming;
  for (const Departure& d : departures) {
    if (departureTime(d) < currentMinute) continue;
    if (!platforms.empty() &&
        (d.platformCode.empty() ||
         std::find(platforms.begin(), platforms.end(), d.platformCode) == platforms.end())) {
      continue;
    }
    upcoming.push_back(&d);
  }
  std::stable_sort(upcoming.begin(), upcoming.end(), [](const Departure* a, const Departure* b) {
    return departureTime(*a) < departureTime(*b);
  });
  std::vector<Departure> selected;
  selected.reserve(std::min(upcoming.size(), limit));
  for (size_t i = 0; i < upcoming.size() && i < limit; i++) {
    selected.push_back(*upcoming[i]);
  }
  return selected;
}
