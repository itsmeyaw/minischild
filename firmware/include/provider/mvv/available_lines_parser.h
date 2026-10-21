#pragma once

#include <ArduinoJson.h>

#include <string>
#include <variant>
#include <vector>

#include "provider/departure_provider.h"

using RouteDirectionsResult = std::variant<std::vector<std::string>, FetchError>;

inline RouteDirectionsResult toRouteDirections(JsonObjectConst root) {
  const char* error = root["error"] | "";
  if (*error) return FetchError{std::string("MVV: ") + error};
  std::vector<std::string> ids;
  for (JsonObjectConst line : root["lines"].as<JsonArrayConst>()) {
    const char* stateless = line["stateless"] | "";
    if (*stateless) ids.push_back(stateless);
  }
  return ids;
}

template <typename TInput>
RouteDirectionsResult parseMvvAvailableLines(TInput&& input) {
  JsonDocument filter;
  filter["error"] = true;
  filter["lines"][0]["stateless"] = true;
  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, input, DeserializationOption::Filter(filter));
  if (err) return FetchError{"MVV JSON: " + std::string(err.c_str())};
  return toRouteDirections(doc.as<JsonObjectConst>());
}
