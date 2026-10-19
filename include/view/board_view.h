#pragma once

#include <ctime>
#include <optional>

#include "model/departure_board.h"

// Full redraws the whole panel (clears e-paper ghosting); Partial updates it without flashing.
enum class Refresh { Partial, Full };

class BoardView {
 public:
  virtual ~BoardView() = default;

  // staleSince is set when board is not fresh: the time of the last successful fetch.
  virtual void render(const DepartureBoard& board, time_t now, std::optional<time_t> staleSince,
                      Refresh refresh) = 0;
  virtual void renderNoConnection(time_t now, Refresh refresh) = 0;
};
