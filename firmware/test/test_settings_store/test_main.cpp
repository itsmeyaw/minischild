#include <unity.h>

#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "settings/settings_store.h"

class FakeBackend : public SettingsBackend {
 public:
  std::optional<std::string> stored;
  std::vector<std::string> writes;
  bool failWrites = false;

  std::optional<std::string> read() override { return stored; }

  bool write(const std::string& document) override {
    if (failWrites) return false;
    writes.push_back(document);
    stored = document;
    return true;
  }
};

static std::string readProtocolFile(const std::string& name) {
  std::ifstream file(std::string(BLE_PROTOCOL_DIR) + "/" + name);
  TEST_ASSERT_TRUE_MESSAGE(file.is_open(), name.c_str());
  std::stringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}

static Settings defaultSettings() {
  return Settings{{"de:09162:2", {"ddb:920S1: :H:j26"}, {}}, "home", "default secret"};
}

static Settings storedSettings() {
  return Settings{{"de:09162:6", {"ddb:920S9: :H:j26"}, {"9"}}, "old", "stored secret"};
}

void setUp() {}
void tearDown() {}

void test_loads_defaults_when_nothing_stored() {
  FakeBackend backend;
  Settings settings = SettingsStore(backend, defaultSettings()).load();
  TEST_ASSERT_EQUAL_STRING("de:09162:2", settings.boardSelection.stationId.c_str());
  TEST_ASSERT_EQUAL_STRING("home", settings.wifiSsid.c_str());
  TEST_ASSERT_EQUAL_STRING("default secret", settings.wifiPassword.c_str());
}

void test_loads_stored_settings_over_defaults() {
  FakeBackend backend;
  backend.stored = encodeSettings(storedSettings(), Password::Include);
  Settings settings = SettingsStore(backend, defaultSettings()).load();
  TEST_ASSERT_EQUAL_STRING("de:09162:6", settings.boardSelection.stationId.c_str());
  TEST_ASSERT_EQUAL_STRING("old", settings.wifiSsid.c_str());
  TEST_ASSERT_EQUAL_STRING("stored secret", settings.wifiPassword.c_str());
}

void test_loads_defaults_when_stored_document_is_corrupt() {
  FakeBackend backend;
  backend.stored = "not json";
  Settings settings = SettingsStore(backend, defaultSettings()).load();
  TEST_ASSERT_EQUAL_STRING("de:09162:2", settings.boardSelection.stationId.c_str());
  TEST_ASSERT_EQUAL_STRING("default secret", settings.wifiPassword.c_str());
}

void test_save_then_load_round_trips_password() {
  FakeBackend backend;
  SettingsStore store(backend, defaultSettings());
  TEST_ASSERT_TRUE(store.save(storedSettings()));
  TEST_ASSERT_EQUAL(1, backend.writes.size());
  Settings settings = store.load();
  TEST_ASSERT_EQUAL_STRING("de:09162:6", settings.boardSelection.stationId.c_str());
  TEST_ASSERT_EQUAL_STRING("old", settings.wifiSsid.c_str());
  TEST_ASSERT_EQUAL_STRING("stored secret", settings.wifiPassword.c_str());
}

void test_applies_valid_write_by_saving_it() {
  FakeBackend backend;
  SettingsStore store(backend, defaultSettings());
  TEST_ASSERT_FALSE(applySettingsWrite(store, readProtocolFile("valid/settings-write.json")));
  TEST_ASSERT_EQUAL(1, backend.writes.size());
  Settings settings = store.load();
  TEST_ASSERT_EQUAL_STRING("de:09162:2", settings.boardSelection.stationId.c_str());
  TEST_ASSERT_EQUAL_STRING("home", settings.wifiSsid.c_str());
  TEST_ASSERT_EQUAL_STRING("correct horse", settings.wifiPassword.c_str());
}

void test_keep_password_write_keeps_stored_password() {
  FakeBackend backend;
  backend.stored = encodeSettings(storedSettings(), Password::Include);
  SettingsStore store(backend, defaultSettings());
  TEST_ASSERT_FALSE(
      applySettingsWrite(store, readProtocolFile("valid/settings-write-keep-password.json")));
  Settings settings = store.load();
  TEST_ASSERT_EQUAL_STRING("de:09162:6", settings.boardSelection.stationId.c_str());
  TEST_ASSERT_EQUAL_STRING("office", settings.wifiSsid.c_str());
  TEST_ASSERT_EQUAL_STRING("stored secret", settings.wifiPassword.c_str());
}

void test_keep_password_write_keeps_default_password_when_nothing_stored() {
  FakeBackend backend;
  SettingsStore store(backend, defaultSettings());
  TEST_ASSERT_FALSE(
      applySettingsWrite(store, readProtocolFile("valid/settings-write-keep-password.json")));
  Settings settings = store.load();
  TEST_ASSERT_EQUAL_STRING("office", settings.wifiSsid.c_str());
  TEST_ASSERT_EQUAL_STRING("default secret", settings.wifiPassword.c_str());
}

void test_rejects_invalid_writes_without_saving() {
  const char* files[] = {"invalid/malformed.json", "invalid/oversized.json",
                         "invalid/missing-station.json", "invalid/wrong-type.json"};
  for (const char* file : files) {
    FakeBackend backend;
    SettingsStore store(backend, defaultSettings());
    TEST_ASSERT_TRUE_MESSAGE(applySettingsWrite(store, readProtocolFile(file)).has_value(), file);
    TEST_ASSERT_EQUAL_MESSAGE(0, backend.writes.size(), file);
    TEST_ASSERT_FALSE_MESSAGE(backend.stored.has_value(), file);
  }
}

void test_reports_failed_save() {
  FakeBackend backend;
  backend.failWrites = true;
  SettingsStore store(backend, defaultSettings());
  std::optional<SettingsError> error =
      applySettingsWrite(store, readProtocolFile("valid/settings-write.json"));
  TEST_ASSERT_TRUE(error.has_value());
  TEST_ASSERT_EQUAL_STRING("could not save settings", error->message.c_str());
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_loads_defaults_when_nothing_stored);
  RUN_TEST(test_loads_stored_settings_over_defaults);
  RUN_TEST(test_loads_defaults_when_stored_document_is_corrupt);
  RUN_TEST(test_save_then_load_round_trips_password);
  RUN_TEST(test_applies_valid_write_by_saving_it);
  RUN_TEST(test_keep_password_write_keeps_stored_password);
  RUN_TEST(test_keep_password_write_keeps_default_password_when_nothing_stored);
  RUN_TEST(test_rejects_invalid_writes_without_saving);
  RUN_TEST(test_reports_failed_save);
  return UNITY_END();
}
