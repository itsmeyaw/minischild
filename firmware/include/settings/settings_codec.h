#pragma once

#include <ArduinoJson.h>

#include <cstddef>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include "settings/settings.h"

constexpr size_t kMaxSettingsDocumentSize = 512;
constexpr size_t kMaxWifiSsidSize = 32;
constexpr size_t kMaxWifiPasswordSize = 64;

struct SettingsError {
  std::string message;
};

using SettingsResult = std::variant<Settings, SettingsError>;

enum class Password { Omit, Include };

namespace settings_codec {

inline bool readStrings(JsonVariantConst value, std::vector<std::string>& out) {
  if (!value.is<JsonArrayConst>()) return false;
  for (JsonVariantConst item : value.as<JsonArrayConst>()) {
    if (!item.is<const char*>()) return false;
    out.push_back(item.as<std::string>());
  }
  return true;
}

inline void writeStrings(JsonArray array, const std::vector<std::string>& strings) {
  for (const std::string& s : strings) array.add(s);
}

inline std::string serialize(const JsonDocument& doc) {
  std::string out;
  serializeJson(doc, out);
  return out;
}

}  // namespace settings_codec

inline SettingsResult decodeSettings(std::string_view json, const Settings& stored) {
  if (json.size() > kMaxSettingsDocumentSize) return SettingsError{"settings over 512 bytes"};
  JsonDocument doc;
  if (deserializeJson(doc, json)) return SettingsError{"settings are not JSON"};
  JsonObjectConst root = doc.as<JsonObjectConst>();
  if (root.isNull()) return SettingsError{"settings are not an object"};

  Settings settings;
  settings.boardSelection.stationId = root["stationId"] | "";
  if (settings.boardSelection.stationId.empty()) return SettingsError{"stationId missing or empty"};
  if (!settings_codec::readStrings(root["routeDirectionIds"],
                                   settings.boardSelection.routeDirectionIds)) {
    return SettingsError{"routeDirectionIds must be an array of strings"};
  }
  if (!settings_codec::readStrings(root["platforms"], settings.boardSelection.platforms)) {
    return SettingsError{"platforms must be an array of strings"};
  }
  settings.wifiSsid = root["wifiSsid"] | "";
  if (settings.wifiSsid.empty() || settings.wifiSsid.size() > kMaxWifiSsidSize) {
    return SettingsError{"wifiSsid must be 1-32 bytes"};
  }
  if (!root["wifiPassword"].isNull() && !root["wifiPassword"].is<const char*>()) {
    return SettingsError{"wifiPassword must be a string"};
  }
  settings.wifiPassword = root["wifiPassword"] | "";
  if (settings.wifiPassword.empty()) settings.wifiPassword = stored.wifiPassword;
  if (settings.wifiPassword.size() > kMaxWifiPasswordSize) {
    return SettingsError{"wifiPassword over 64 bytes"};
  }
  return settings;
}

inline std::string encodeSettings(const Settings& settings, Password password = Password::Omit) {
  JsonDocument doc;
  doc["stationId"] = settings.boardSelection.stationId;
  settings_codec::writeStrings(doc["routeDirectionIds"].to<JsonArray>(),
                               settings.boardSelection.routeDirectionIds);
  settings_codec::writeStrings(doc["platforms"].to<JsonArray>(), settings.boardSelection.platforms);
  doc["wifiSsid"] = settings.wifiSsid;
  if (password == Password::Include) doc["wifiPassword"] = settings.wifiPassword;
  return settings_codec::serialize(doc);
}

inline std::string encodeInfo(const std::string& provider, const std::string& firmware) {
  JsonDocument doc;
  doc["provider"] = provider;
  doc["firmware"] = firmware;
  return settings_codec::serialize(doc);
}
