#pragma once

#include <ArduinoJson.h>

#include <algorithm>
#include <ctime>
#include <optional>
#include <string>
#include <vector>

#include "provider/bvg/iso8601.h"
#include "provider/departure_provider.h"

// Route Direction ID: "<line name>|<destination station IBNR>", the destination empty when unknown.
inline std::string bvgRouteDirectionId(const std::string& lineName,
                                       const std::string& destinationId) {
  return lineName + "|" + destinationId;
}

inline FetchResult toBvgDepartureBoard(JsonObjectConst root, time_t now,
                                       const std::vector<std::string>& routeDirectionIds) {
  DepartureBoard board;
  board.timestamp = now;
  const char* message = root["message"] | "";
  if (*message) return FetchError{std::string("BVG: ") + message};
  board.stopName = root["departures"][0]["stop"]["name"] | "";
  for (JsonObjectConst dep : root["departures"].as<JsonArrayConst>()) {
    if (dep["cancelled"] | false) continue;
    std::string routeDirectionId =
        bvgRouteDirectionId(dep["line"]["name"] | "", dep["destination"]["id"] | "");
    if (!routeDirectionIds.empty() && std::find(routeDirectionIds.begin(), routeDirectionIds.end(),
                                                routeDirectionId) == routeDirectionIds.end()) {
      continue;
    }
    std::optional<time_t> planned = parseIso8601(dep["plannedWhen"] | "");
    if (!planned) continue;
    Departure d{};
    d.routeShortName = dep["line"]["name"] | "";
    d.tripHeadsign = dep["direction"] | "";
    d.platformCode = dep["platform"] | "";
    if (d.platformCode.empty()) d.platformCode = dep["plannedPlatform"] | "";
    d.scheduledDepartureTime = *planned;
    d.scheduleRelationship = ScheduleRelationship::NoData;
    std::optional<time_t> when = parseIso8601(dep["when"] | "");
    if (dep["delay"].is<int32_t>() && when) {
      d.scheduleRelationship = ScheduleRelationship::Scheduled;
      d.departure = StopTimeEvent{*when, dep["delay"].as<int32_t>()};
    }
    board.departures.push_back(d);
  }
  return board;
}

template <typename TInput>
FetchResult parseBvgDepartures(TInput&& input, time_t now,
                               const std::vector<std::string>& routeDirectionIds) {
  JsonDocument filter;
  filter["message"] = true;
  filter["departures"][0]["cancelled"] = true;
  filter["departures"][0]["stop"]["name"] = true;
  filter["departures"][0]["line"]["name"] = true;
  filter["departures"][0]["destination"]["id"] = true;
  filter["departures"][0]["direction"] = true;
  filter["departures"][0]["platform"] = true;
  filter["departures"][0]["plannedPlatform"] = true;
  filter["departures"][0]["plannedWhen"] = true;
  filter["departures"][0]["when"] = true;
  filter["departures"][0]["delay"] = true;
  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, input, DeserializationOption::Filter(filter));
  if (err) return FetchError{"BVG JSON: " + std::string(err.c_str())};
  return toBvgDepartureBoard(doc.as<JsonObjectConst>(), now, routeDirectionIds);
}
