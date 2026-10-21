#pragma once

#include <ArduinoJson.h>

#include <string>
#include <variant>

#include "provider/departure_provider.h"

using StopNameResult = std::variant<std::string, FetchError>;

inline StopNameResult toStopName(JsonObjectConst root, const std::string& stationId) {
  for (JsonObjectConst location : root["locations"].as<JsonArrayConst>()) {
    if (stationId == (location["id"] | "")) {
      for (JsonObjectConst stop : location["assignedStops"].as<JsonArrayConst>()) {
        if (stationId == (stop["id"] | "")) return std::string(stop["name"] | "");
      }
      return std::string(location["name"] | "");
    }
  }
  return FetchError{"MVV: no stop named " + stationId};
}

template <typename TInput>
StopNameResult parseMvvStopName(TInput&& input, const std::string& stationId) {
  JsonDocument filter;
  filter["locations"][0]["id"] = true;
  filter["locations"][0]["name"] = true;
  filter["locations"][0]["assignedStops"][0]["id"] = true;
  filter["locations"][0]["assignedStops"][0]["name"] = true;
  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, input, DeserializationOption::Filter(filter));
  if (err) return FetchError{"MVV JSON: " + std::string(err.c_str())};
  return toStopName(doc.as<JsonObjectConst>(), stationId);
}
