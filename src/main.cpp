#include <Arduino.h>
#include <WiFi.h>

#include <ctime>
#include <utility>
#include <variant>

#include "board/board_selection.h"
#include "config.h"
#include "setup.h"

namespace {
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
  BoardView& view = setupBoardView();
  DepartureProvider& provider = setupDepartureProvider();
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
