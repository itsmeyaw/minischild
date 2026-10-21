#pragma once

#include <ArduinoJson.h>

#include <cstring>
#include <ctime>
#include <optional>
#include <string>
#include <vector>

#include "provider/departure_provider.h"
#include "provider/route_direction.h"
#include "util/iso8601.h"

// Any realtimeStatus naming a cancellation (TRIP_CANCELLED in the verified responses) drops the
// event.
inline bool isCancelled(JsonArrayConst realtimeStatus) {
  for (JsonVariantConst status : realtimeStatus) {
    if (strstr(status | "", "CANCELLED")) return true;
  }
  return false;
}

inline FetchResult toEfaDepartureBoard(JsonObjectConst root, time_t now,
                                       const std::vector<std::string>& routeDirectionIds) {
  DepartureBoard board;
  board.timestamp = now;
  JsonObjectConst station = root["locations"][0];
  board.stopName = station["disassembledName"] | (station["name"] | "");
  for (JsonObjectConst e : root["stopEvents"].as<JsonArrayConst>()) {
    if (isCancelled(e["realtimeStatus"].as<JsonArrayConst>())) continue;
    JsonObjectConst transportation = e["transportation"];
    std::string line = transportation["disassembledName"] | (transportation["number"] | "");
    std::optional<time_t> planned = parseIso8601(e["departureTimePlanned"] | "");
    if (!planned) continue;
    JsonObjectConst props = e["location"]["properties"];
    Departure d{};
    d.routeShortName = line;
    d.tripHeadsign = transportation["destination"]["name"] | "";
    d.platformCode = props["platformName"] | "";
    if (d.platformCode.empty()) d.platformCode = props["plannedPlatformName"] | "";
    if (d.platformCode.empty()) d.platformCode = props["platform"] | "";
    d.scheduledDepartureTime = *planned;
    d.scheduleRelationship = ScheduleRelationship::NoData;
    std::optional<time_t> estimated = parseIso8601(e["departureTimeEstimated"] | "");
    if (estimated) {
      d.scheduleRelationship = ScheduleRelationship::Scheduled;
      d.departure = StopTimeEvent{*estimated, static_cast<int32_t>(*estimated - *planned)};
    }
    board.departures.push_back(d);
  }
  return board;
}

template <typename TInput>
FetchResult parseEfaDepartures(TInput&& input, time_t now,
                               const std::vector<std::string>& routeDirectionIds) {
  JsonDocument filter;
  filter["locations"][0]["name"] = true;
  filter["locations"][0]["disassembledName"] = true;
  filter["stopEvents"][0]["departureTimePlanned"] = true;
  filter["stopEvents"][0]["departureTimeEstimated"] = true;
  filter["stopEvents"][0]["realtimeStatus"] = true;
  filter["stopEvents"][0]["location"]["properties"]["platformName"] = true;
  filter["stopEvents"][0]["location"]["properties"]["plannedPlatformName"] = true;
  filter["stopEvents"][0]["location"]["properties"]["platform"] = true;
  filter["stopEvents"][0]["transportation"]["disassembledName"] = true;
  filter["stopEvents"][0]["transportation"]["number"] = true;
  filter["stopEvents"][0]["transportation"]["destination"]["id"] = true;
  filter["stopEvents"][0]["transportation"]["destination"]["name"] = true;
  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, input, DeserializationOption::Filter(filter));
  if (err) return FetchError{"EFA JSON: " + std::string(err.c_str())};
  return toEfaDepartureBoard(doc.as<JsonObjectConst>(), now, routeDirectionIds);
}
