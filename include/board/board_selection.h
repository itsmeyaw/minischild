#pragma once

#include <algorithm>
#include <cstddef>
#include <ctime>
#include <vector>

#include "model/departure_board.h"

// Upcoming Departures soonest first, at most limit. A Departure in the current minute still
// counts as upcoming, matching countdownLabel's "now".
inline std::vector<Departure> selectDepartures(const std::vector<Departure>& departures, time_t now,
                                               size_t limit) {
  const time_t currentMinute = now - now % 60;
  std::vector<const Departure*> upcoming;
  for (const Departure& d : departures) {
    if (departureTime(d) >= currentMinute) upcoming.push_back(&d);
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
