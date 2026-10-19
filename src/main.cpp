#include <Arduino.h>
#include <GxEPD2_BW.h>
#include <SPI.h>
#include <WiFi.h>

#include <ctime>
#include <iterator>
#include <utility>
#include <variant>

#include "board/board_selection.h"
#include "config.h"
#include "provider/mvv/departure_provider.h"
#include "view/board_view.h"
#include "view/epaper_290/epaper_290_board_view.h"

namespace {
Epaper290BoardView::Display display(GxEPD2_290_T94_V2(PIN_CS, PIN_DC, PIN_RST, PIN_BUSY));
Epaper290BoardView epaperView(display);
MvvDepartureProvider mvvProvider(STATION_ID,
                                 {std::begin(ROUTE_DIRECTION_IDS), std::end(ROUTE_DIRECTION_IDS)});

bool connectWifi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  const unsigned long deadline = millis() + 20000;
  while (WiFi.status() != WL_CONNECTED) {
    if (millis() > deadline) return false;
    delay(250);
  }
  return true;
}

bool syncClock() {
  configTzTime("CET-1CEST,M3.5.0,M10.5.0/3", "pool.ntp.org", "time.nist.gov");
  tm t;
  return getLocalTime(&t, 10000);
}
}  // namespace

void setup() {
  Serial.begin(115200);
  SPI.begin(PIN_CLK, -1, PIN_DIN, PIN_CS);
  epaperView.begin();

  BoardView& view = epaperView;
  DepartureProvider& provider = mvvProvider;
  if (!connectWifi() || !syncClock()) {
    Serial.println("WiFi or clock sync failed");
    view.renderNoConnection(time(nullptr));
    return;
  }

  FetchResult result = provider.fetch();
  if (auto* error = std::get_if<FetchError>(&result)) {
    Serial.println(error->message.c_str());
    view.renderNoConnection(time(nullptr));
    return;
  }
  if (auto* board = std::get_if<DepartureBoard>(&result)) {
    const time_t now = time(nullptr);
    board->departures = selectDepartures(std::move(board->departures), now);
    view.render(*board, now);
  }
}

void loop() {}
