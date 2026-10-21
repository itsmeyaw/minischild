#pragma once

#include <string>

#include "board/board_selection.h"

// A Device's Board Selection and the WiFi network it joins.
struct Settings {
  BoardSelection boardSelection;
  std::string wifiSsid;
  std::string wifiPassword;
};
