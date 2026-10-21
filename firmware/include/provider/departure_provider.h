#pragma once

#include <string>
#include <variant>

#include "board/board_selection.h"
#include "model/departure_board.h"

struct FetchError {
  std::string message;
};

using FetchResult = std::variant<DepartureBoard, FetchError>;

class DepartureProvider {
 public:
  virtual ~DepartureProvider() = default;
  virtual FetchResult fetch() = 0;
  // Shows the given Station and Route Directions from the next fetch on.
  virtual void select(const BoardSelection& selection) = 0;
};
