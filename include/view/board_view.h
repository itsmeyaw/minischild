#pragma once

#include <ctime>

#include "model/departure_board.h"

class BoardView {
 public:
  virtual ~BoardView() = default;

  virtual void render(const DepartureBoard& board, time_t now) = 0;
  virtual void renderNoConnection(time_t now) = 0;
};
