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
      : info_(std::move(info)), passkey_(passkey), settingsCallbacks_(store, saved_) {}

  void begin();
  // True once per valid write, so loop() handles each saved write exactly once.
  bool takeSettingsSaved() { return saved_.exchange(false); }

 private:
  class SettingsCallbacks : public NimBLECharacteristicCallbacks {
   public:
    SettingsCallbacks(SettingsStore& store, std::atomic<bool>& saved)
        : store_(store), saved_(saved) {}
    void onRead(NimBLECharacteristic* characteristic, NimBLEConnInfo& connInfo) override;
    void onWrite(NimBLECharacteristic* characteristic, NimBLEConnInfo& connInfo) override;

   private:
    SettingsStore& store_;
    std::atomic<bool>& saved_;
  };

  std::string info_;
  uint32_t passkey_;
  std::atomic<bool> saved_{false};
  SettingsCallbacks settingsCallbacks_;
};
