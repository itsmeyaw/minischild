#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <variant>

#include "settings/settings.h"
#include "settings/settings_codec.h"

// Raw non-volatile storage of the one settings document.
class SettingsBackend {
 public:
  virtual ~SettingsBackend() = default;
  // The stored document, or nullopt when nothing is stored.
  virtual std::optional<std::string> read() = 0;
  // False when the document could not be stored.
  virtual bool write(const std::string& document) = 0;
};

// Settings as the Device uses them: stored settings over config.h defaults.
class SettingsStore {
 public:
  SettingsStore(SettingsBackend& backend, Settings defaults)
      : backend_(backend), defaults_(std::move(defaults)) {}

  Settings load() const {
    std::optional<std::string> stored = backend_.read();
    if (!stored) return defaults_;
    SettingsResult result = decodeSettings(*stored, defaults_);
    if (std::holds_alternative<SettingsError>(result)) return defaults_;
    return std::get<Settings>(result);
  }

  bool save(const Settings& settings) {
    return backend_.write(encodeSettings(settings, Password::Include));
  }

 private:
  SettingsBackend& backend_;
  Settings defaults_;
};

// Validates a settings write from BLE against the stored settings and saves it.
// An empty password keeps the effective one. Returns the error, or nullopt when saved.
inline std::optional<SettingsError> applySettingsWrite(SettingsStore& store,
                                                       std::string_view json) {
  SettingsResult result = decodeSettings(json, store.load());
  if (std::holds_alternative<SettingsError>(result)) return std::get<SettingsError>(result);
  if (!store.save(std::get<Settings>(result))) return SettingsError{"could not save settings"};
  return std::nullopt;
}
