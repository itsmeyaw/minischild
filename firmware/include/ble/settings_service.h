#pragma once

#include <NimBLECharacteristic.h>

#include <atomic>
#include <cstdint>
#include <string>
#include <utility>

#include "settings/settings_store.h"

// Serves the GATT contract in docs/ble-protocol.md: the info and settings characteristics.
class BleSettingsService {
 public:
  BleSettingsService(SettingsStore& store, std::string info, uint32_t passkey)
      : info_(std::move(info)), passkey_(passkey), settingsCallbacks_(store, restart_) {}

  void begin();
  bool restartRequested() const { return restart_; }

 private:
  class SettingsCallbacks : public NimBLECharacteristicCallbacks {
   public:
    SettingsCallbacks(SettingsStore& store, std::atomic<bool>& restart)
        : store_(store), restart_(restart) {}
    void onRead(NimBLECharacteristic* characteristic, NimBLEConnInfo& connInfo) override;
    void onWrite(NimBLECharacteristic* characteristic, NimBLEConnInfo& connInfo) override;

   private:
    SettingsStore& store_;
    std::atomic<bool>& restart_;
  };

  std::string info_;
  uint32_t passkey_;
  std::atomic<bool> restart_{false};
  SettingsCallbacks settingsCallbacks_;
};
