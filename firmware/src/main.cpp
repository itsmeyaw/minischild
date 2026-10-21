#include <Arduino.h>
#include <WiFi.h>

#include <ctime>
#include <optional>

#include "board/board_controller.h"
#include "board/clock.h"
#include "board/connection_watchdog.h"
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

class SystemClock : public Clock {
 public:
  time_t now() override { return time(nullptr); }
};

SystemClock systemClock;
std::optional<BoardController> controller;
ConnectionWatchdog watchdog;
}  // namespace

void setup() {
  Serial.begin(115200);
  BoardView& view = setupBoardView();
  DepartureProvider& provider = setupDepartureProvider();
  // Both run even on failure: the watchdog retries WiFi and SNTP keeps polling, so fetches recover.
  if (!connectWifi()) Serial.println("WiFi connect failed");
  if (!syncClock()) Serial.println("Clock sync failed");
  controller.emplace(provider, view, systemClock);
}

void loop() {
  // The core's auto-reconnect gives up after some disconnect reasons, so retry here too.
  switch (watchdog.check(WiFi.status() == WL_CONNECTED, millis())) {
    case WatchdogAction::Reconnect:
      Serial.println("WiFi down, reconnecting");
      WiFi.reconnect();
      break;
    case WatchdogAction::Restart:
      Serial.println("WiFi down too long, restarting");
      ESP.restart();
      break;
    case WatchdogAction::None:
      break;
  }
  // Heap figures tell a fragmented heap (TLS needs two ~16 KB blocks) from a network error.
  if (std::optional<FetchError> error = controller->tick()) {
    Serial.printf("%s (free heap %u, largest block %u)\n", error->message.c_str(),
                  static_cast<unsigned>(ESP.getFreeHeap()),
                  static_cast<unsigned>(ESP.getMaxAllocHeap()));
  }
  delay(1000);
}
