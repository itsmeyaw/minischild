#pragma once

#include "board/board_selection.h"
#include "provider/departure_provider.h"
#include "view/board_view.h"

// Each build links exactly one implementation of each, picked by scripts/select_modules.py.
// See docs/adr/0003-compile-time-view-and-provider-selection.md.

// Initialises the display hardware and returns the view drawing on it.
BoardView& setupBoardView();

// Returns the provider for selection's Station and Route Directions.
DepartureProvider& setupDepartureProvider(const BoardSelection& selection);
