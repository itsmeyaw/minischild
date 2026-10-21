#pragma once

#include <functional>
#include <optional>
#include <string>
#include <utility>

#include "provider/mvv/stop_name_parser.h"

// The Station's name, looked up once. A failed lookup gives "" and is retried on the next get.
class StopNameCache {
 public:
  explicit StopNameCache(std::function<StopNameResult()> lookup) : lookup_(std::move(lookup)) {}

  std::string get() {
    if (!cache_) {
      StopNameResult looked = lookup_();
      if (const std::string* name = std::get_if<std::string>(&looked)) cache_ = *name;
    }
    return cache_.value_or("");
  }

 private:
  std::function<StopNameResult()> lookup_;
  std::optional<std::string> cache_;
};
