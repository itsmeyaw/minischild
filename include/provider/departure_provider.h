#pragma once

#include <string>
#include <variant>

#include "model/departure_board.h"

struct FetchError {
  std::string message;
};

using FetchResult = std::variant<DepartureBoard, FetchError>;

class DepartureProvider {
 public:
  virtual ~DepartureProvider() = default;
  virtual FetchResult fetch() = 0;
};
