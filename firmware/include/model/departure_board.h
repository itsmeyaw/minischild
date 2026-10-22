#pragma once

#include <cstdint>
#include <ctime>
#include <optional>
#include <string>
#include <vector>

// GTFS Realtime-shaped model, see docs/adr/0002-gtfs-realtime-shaped-model.md.

enum class ScheduleRelationship { Scheduled, NoData, Skipped };

struct StopTimeEvent {
  time_t time;
  int32_t delay;  // seconds, positive means late
};

struct Departure {
  std::string routeShortName;
  std::string tripHeadsign;
  std::string platformCode;
  time_t scheduledDepartureTime;
  ScheduleRelationship scheduleRelationship;
  std::optional<StopTimeEvent> departure;
};

inline time_t departureTime(const Departure& d) {
  return d.departure ? d.departure->time : d.scheduledDepartureTime;
}

struct DepartureBoard {
  std::string stopName;
  time_t timestamp;
  std::vector<Departure> departures;
};
