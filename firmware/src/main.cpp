#include <Arduino.h>
#include <WiFi.h>

#include <ctime>
#include <optional>

#include "ble/settings_service.h"
#include "board/board_controller.h"
#include "board/clock.h"
#include "board/connection_watchdog.h"
#include "config.h"
#include "settings/nvs_settings_backend.h"
#include "settings/settings_store.h"
#include "setup.h"

static_assert(BLE_PASSKEY <= 999999, "BLE_PASSKEY must have at most 6 digits");

namespace {
bool connectWifi(const std::string& ssid, const std::string& password) {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), password.c_str());
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
NvsSettingsBackend settingsBackend;
std::optional<SettingsStore> settingsStore;
std::optional<BoardController> controller;
std::optional<BleSettingsService> bleService;
ConnectionWatchdog watchdog;
uint32_t lowestMinFreeHeap = UINT32_MAX;
}  // namespace

void setup() {
  Serial.begin(115200);
  settingsStore.emplace(
      settingsBackend,
      Settings{{STATION_ID, ROUTE_DIRECTION_IDS, PLATFORMS}, WIFI_SSID, WIFI_PASSWORD});
  const Settings settings = settingsStore->load();
  // Before WiFi, so a Device with the wrong WiFi can still be reconfigured over BLE.
  bleService.emplace(*settingsStore, encodeInfo(PROVIDER_NAME, FIRMWARE_VERSION), BLE_PASSKEY);
  bleService->begin();
  BoardView& view = setupBoardView();
  DepartureProvider& provider = setupDepartureProvider(settings.boardSelection);
  // Both run even on failure: the watchdog retries WiFi and SNTP keeps polling, so fetches recover.
  if (!connectWifi(settings.wifiSsid, settings.wifiPassword)) {
    Serial.println("WiFi connect failed");
  }
  if (!syncClock()) Serial.println("Clock sync failed");
  controller.emplace(provider, view, systemClock, settings.boardSelection.platforms);
}

void loop() {
  // Lets the write response reach the phone before the link drops.
  if (bleService->restartRequested()) {
    Serial.println("Restarting to apply settings");
    delay(500);
    ESP.restart();
  }
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
  // BLE shares the heap with TLS; a new low tells whether the budget in ADR 0006 holds.
  const uint32_t minFreeHeap = ESP.getMinFreeHeap();
  if (minFreeHeap < lowestMinFreeHeap) {
    lowestMinFreeHeap = minFreeHeap;
    Serial.printf("Minimum free heap %u\n", static_cast<unsigned>(minFreeHeap));
  }
  delay(1000);
}
