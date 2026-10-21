#include "ble/settings_service.h"

#include <Arduino.h>
#include <NimBLEDevice.h>
#include <esp_mac.h>

#include <cstdio>
#include <optional>
#include <string_view>

#include "settings/settings_codec.h"

namespace {
constexpr const char* kServiceUuid = "d2c3f200-ea7c-4ae4-bc29-8b05a10aa4c7";
constexpr const char* kInfoUuid = "d2c3f201-ea7c-4ae4-bc29-8b05a10aa4c7";
constexpr const char* kSettingsUuid = "d2c3f202-ea7c-4ae4-bc29-8b05a10aa4c7";

constexpr uint32_t kEncryptedRead =
    NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::READ_ENC | NIMBLE_PROPERTY::READ_AUTHEN;
constexpr uint32_t kEncryptedWrite = kEncryptedRead | NIMBLE_PROPERTY::WRITE |
                                     NIMBLE_PROPERTY::WRITE_ENC | NIMBLE_PROPERTY::WRITE_AUTHEN;
}  // namespace

void BleSettingsService::begin() {
  uint8_t mac[6];
  esp_read_mac(mac, ESP_MAC_BT);
  char name[sizeof("Minischild-XXXX")];
  snprintf(name, sizeof(name), "Minischild-%02X%02X", mac[4], mac[5]);

  NimBLEDevice::init(name);
  NimBLEDevice::setSecurityAuth(/*bonding=*/true, /*mitm=*/true, /*sc=*/true);
  NimBLEDevice::setSecurityIOCap(BLE_HS_IO_DISPLAY_ONLY);
  NimBLEDevice::setSecurityPasskey(passkey_);

  NimBLEServer* server = NimBLEDevice::createServer();
  server->advertiseOnDisconnect(true);
  NimBLEService* service = server->createService(kServiceUuid);
  service->createCharacteristic(kInfoUuid, kEncryptedRead)->setValue(info_);
  NimBLECharacteristic* settings =
      service->createCharacteristic(kSettingsUuid, kEncryptedWrite, kMaxSettingsDocumentSize);
  settings->setCallbacks(&settingsCallbacks_);

  // The 128-bit service UUID and the name don't both fit in 31 bytes, so the name goes in the scan
  // response.
  NimBLEAdvertising* advertising = NimBLEDevice::getAdvertising();
  advertising->addServiceUUID(kServiceUuid);
  advertising->enableScanResponse(true);
  advertising->setName(name);
  advertising->start();
}

void BleSettingsService::SettingsCallbacks::onRead(NimBLECharacteristic* characteristic,
                                                   NimBLEConnInfo&) {
  characteristic->setValue(encodeSettings(store_.load()));
}

void BleSettingsService::SettingsCallbacks::onWrite(NimBLECharacteristic* characteristic,
                                                    NimBLEConnInfo&) {
  std::string json = characteristic->getValue();
  if (std::optional<SettingsError> error = applySettingsWrite(store_, json)) {
    Serial.printf("Settings write rejected: %s\n", error->message.c_str());
  } else {
    Serial.println("Settings saved");
    saved_ = true;
  }
  // Replace the written value so the WiFi password doesn't stay readable in the attribute.
  characteristic->setValue(encodeSettings(store_.load()));
}
