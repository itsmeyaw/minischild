#pragma once

#include <functional>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "provider/mvv/available_lines_parser.h"

// The Route Directions to request: the configured ones, or, when none are configured, all of
// the Station's, looked up once.
class RouteDirections {
 public:
  RouteDirections(std::vector<std::string> configured,
                  std::function<RouteDirectionsResult()> lookup)
      : configured_(std::move(configured)), lookup_(std::move(lookup)) {}

  RouteDirectionsResult get() {
    if (!configured_.empty()) return configured_;
    if (!cache_) {
      RouteDirectionsResult looked = lookup_();
      if (std::holds_alternative<FetchError>(looked)) return looked;
      cache_ = std::get<std::vector<std::string>>(looked);
    }
    return *cache_;
  }

 private:
  std::vector<std::string> configured_;
  std::function<RouteDirectionsResult()> lookup_;
  std::optional<std::vector<std::string>> cache_;
};
