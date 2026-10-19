#pragma once

#include <ArduinoJson.h>

#include <cstdio>
#include <ctime>
#include <string>

#include "provider/departure_provider.h"

inline FetchResult toDepartureBoard(JsonObjectConst root, time_t now) {
  const char* error = root["error"] | "";
  if (*error) return FetchError{std::string("MVV: ") + error};
  DepartureBoard board;
  board.timestamp = now;
  board.stopName = root["departures"][0]["station"]["name"] | "";
  for (JsonObjectConst dep : root["departures"].as<JsonArrayConst>()) {
    int year, month, day, hour, minute, liveHour, liveMinute;
    if (sscanf(dep["departureDate"] | "", "%4d%2d%2d", &year, &month, &day) != 3 ||
        sscanf(dep["departurePlanned"] | "", "%d:%d", &hour, &minute) != 2) {
      continue;
    }
    // The process TZ is Europe/Berlin (set in main.cpp), so mktime reads these as local time.
    tm scheduled{};
    scheduled.tm_year = year - 1900;
    scheduled.tm_mon = month - 1;
    scheduled.tm_mday = day;
    scheduled.tm_hour = hour;
    scheduled.tm_min = minute;
    scheduled.tm_isdst = -1;
    Departure d{};
    d.routeShortName = dep["line"]["number"] | "";
    d.tripHeadsign = dep["direction"] | "";
    d.platformCode = dep["track"] | "";
    d.scheduledDepartureTime = mktime(&scheduled);
    d.scheduleRelationship = ScheduleRelationship::NoData;
    if (sscanf(dep["departureLive"] | "", "%d:%d", &liveHour, &liveMinute) == 2) {
      tm live = scheduled;
      live.tm_hour = liveHour;
      live.tm_min = liveMinute;
      live.tm_isdst = -1;
      time_t liveTime = mktime(&live);
      // The live time has no date: a 23:59 Departure delayed to 00:03 is on the next day.
      constexpr time_t kHalfDay = 12 * 60 * 60;
      if (liveTime - d.scheduledDepartureTime > kHalfDay) live.tm_mday -= 1;
      if (d.scheduledDepartureTime - liveTime > kHalfDay) live.tm_mday += 1;
      liveTime = mktime(&live);
      d.scheduleRelationship = ScheduleRelationship::Scheduled;
      d.departure =
          StopTimeEvent{liveTime, static_cast<int32_t>(liveTime - d.scheduledDepartureTime)};
    }
    board.departures.push_back(d);
  }
  return board;
}

template <typename TInput>
FetchResult parseMvvDepartures(TInput&& input, time_t now) {
  JsonDocument filter;
  filter["error"] = true;
  filter["departures"][0]["line"]["number"] = true;
  filter["departures"][0]["direction"] = true;
  filter["departures"][0]["station"]["name"] = true;
  filter["departures"][0]["track"] = true;
  filter["departures"][0]["departureDate"] = true;
  filter["departures"][0]["departurePlanned"] = true;
  filter["departures"][0]["departureLive"] = true;
  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, input, DeserializationOption::Filter(filter));
  if (err) return FetchError{"MVV JSON: " + std::string(err.c_str())};
  return toDepartureBoard(doc.as<JsonObjectConst>(), now);
}
