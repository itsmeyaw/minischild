#include "settings/nvs_settings_backend.h"

#include <Preferences.h>

namespace {
constexpr const char* kNamespace = "minischild";
constexpr const char* kKey = "settings";
}  // namespace

std::optional<std::string> NvsSettingsBackend::read() {
  Preferences prefs;
  // Fails on first boot, before the namespace exists.
  if (!prefs.begin(kNamespace, /*readOnly=*/true)) return std::nullopt;
  std::optional<std::string> document;
  if (prefs.isKey(kKey)) document = prefs.getString(kKey).c_str();
  prefs.end();
  return document;
}

bool NvsSettingsBackend::write(const std::string& document) {
  Preferences prefs;
  if (document.empty() || !prefs.begin(kNamespace)) return false;
  // putString returns the bytes written, so a short write shows up as a mismatch.
  const size_t written = prefs.putString(kKey, document.c_str());
  prefs.end();
  return written == document.size();
}
