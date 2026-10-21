#pragma once

#include <optional>
#include <string>

#include "settings/settings_store.h"

// Keeps the settings document in the ESP32's NVS flash, where it survives power loss.
class NvsSettingsBackend : public SettingsBackend {
 public:
  std::optional<std::string> read() override;
  bool write(const std::string& document) override;
};
