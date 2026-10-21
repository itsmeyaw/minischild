#pragma once

#include <string>

#include "board/board_selection.h"

// A Device's Board Selection and the WiFi network it joins.
struct Settings {
  BoardSelection boardSelection;
  std::string wifiSsid;
  std::string wifiPassword;
};

// What a saved write changes on the running Device. Joining another WiFi needs a restart, which
// also applies the Board Selection.
enum class SettingsChange { None, BoardSelection, Wifi };

inline SettingsChange settingsChange(const Settings& running, const Settings& saved) {
  if (saved.wifiSsid != running.wifiSsid || saved.wifiPassword != running.wifiPassword) {
    return SettingsChange::Wifi;
  }
  const BoardSelection& was = running.boardSelection;
  const BoardSelection& is = saved.boardSelection;
  if (is.stationId != was.stationId || is.routeDirectionIds != was.routeDirectionIds ||
      is.platforms != was.platforms) {
    return SettingsChange::BoardSelection;
  }
  return SettingsChange::None;
}
