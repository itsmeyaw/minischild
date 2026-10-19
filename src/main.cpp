#include <Arduino.h>
#include <GxEPD2_BW.h>
#include <SPI.h>

#include <cstdlib>
#include <ctime>

#include "board_view.h"
#include "config.h"
#include "epaper_board_view.h"

namespace {
EpaperBoardView::Display display(GxEPD2_290_T94_V2(PIN_CS, PIN_DC, PIN_RST, PIN_BUSY));
EpaperBoardView epaperView(display);
}  // namespace

void setup() {
  setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 1);
  tzset();

  const time_t now = 1792411200;  // 2026-10-19 12:00:00 UTC

  DepartureBoard board;
  board.stopName = "Münchner Freiheit";
  board.timestamp = now;
  board.departures = {
      {"U6", "Garching-Forschungszentrum", "", now + 4 * 60, ScheduleRelationship::Scheduled,
       std::nullopt},
      {"U6", "Großhadern", "", now + 9 * 60, ScheduleRelationship::Scheduled,
       StopTimeEvent{now + 12 * 60, 180}},
      {"19", "Fürstenried West", "", now, ScheduleRelationship::Scheduled, std::nullopt},
      {"U1", "Mangfallplatz", "", now + 14 * 60, ScheduleRelationship::NoData, std::nullopt},
  };

  SPI.begin(PIN_CLK, -1, PIN_DIN, PIN_CS);
  epaperView.begin();

  BoardView& view = epaperView;
  view.render(board, now);
}

void loop() {}
