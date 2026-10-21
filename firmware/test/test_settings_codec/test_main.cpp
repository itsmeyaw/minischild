#include <unity.h>

#include <fstream>
#include <sstream>
#include <string>

#include "settings/settings_codec.h"

static std::string readProtocolFile(const std::string& name) {
  std::ifstream file(std::string(BLE_PROTOCOL_DIR) + "/" + name);
  TEST_ASSERT_TRUE_MESSAGE(file.is_open(), name.c_str());
  std::stringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}

static bool sameJson(const std::string& a, const std::string& b) {
  JsonDocument docA;
  JsonDocument docB;
  TEST_ASSERT_FALSE(deserializeJson(docA, a));
  TEST_ASSERT_FALSE(deserializeJson(docB, b));
  return docA.as<JsonObjectConst>() == docB.as<JsonObjectConst>();
}

static Settings storedSettings() {
  return Settings{{"de:09162:6", {"ddb:920S9: :H:j26"}, {"9"}}, "old", "stored secret"};
}

static Settings decodeOrFail(const std::string& json, const Settings& stored) {
  SettingsResult result = decodeSettings(json, stored);
  TEST_ASSERT_TRUE_MESSAGE(std::holds_alternative<Settings>(result), json.c_str());
  return std::get<Settings>(result);
}

static void assertRejected(const std::string& json) {
  SettingsResult result = decodeSettings(json, storedSettings());
  TEST_ASSERT_TRUE_MESSAGE(std::holds_alternative<SettingsError>(result), json.c_str());
}

void setUp() {}
void tearDown() {}

void test_decodes_settings_write_with_password() {
  Settings settings = decodeOrFail(readProtocolFile("valid/settings-write.json"), storedSettings());
  TEST_ASSERT_EQUAL_STRING("de:09162:2", settings.boardSelection.stationId.c_str());
  TEST_ASSERT_EQUAL(2, settings.boardSelection.routeDirectionIds.size());
  TEST_ASSERT_EQUAL_STRING("ddb:920S1: :R:j26",
                           settings.boardSelection.routeDirectionIds[1].c_str());
  TEST_ASSERT_EQUAL(2, settings.boardSelection.platforms.size());
  TEST_ASSERT_EQUAL_STRING("2", settings.boardSelection.platforms[1].c_str());
  TEST_ASSERT_EQUAL_STRING("home", settings.wifiSsid.c_str());
  TEST_ASSERT_EQUAL_STRING("correct horse", settings.wifiPassword.c_str());
}

void test_missing_password_keeps_stored_password() {
  Settings settings =
      decodeOrFail(readProtocolFile("valid/settings-write-keep-password.json"), storedSettings());
  TEST_ASSERT_EQUAL_STRING("de:09162:6", settings.boardSelection.stationId.c_str());
  TEST_ASSERT_EQUAL(0, settings.boardSelection.routeDirectionIds.size());
  TEST_ASSERT_EQUAL(0, settings.boardSelection.platforms.size());
  TEST_ASSERT_EQUAL_STRING("office", settings.wifiSsid.c_str());
  TEST_ASSERT_EQUAL_STRING("stored secret", settings.wifiPassword.c_str());
}

void test_empty_password_keeps_stored_password() {
  Settings settings = decodeOrFail(
      R"({"stationId": "de:09162:6", "routeDirectionIds": [], "platforms": [], "wifiSsid": "office", "wifiPassword": ""})",
      storedSettings());
  TEST_ASSERT_EQUAL_STRING("stored secret", settings.wifiPassword.c_str());
}

void test_rejects_malformed_json() { assertRejected(readProtocolFile("invalid/malformed.json")); }

void test_rejects_oversized_document() {
  assertRejected(readProtocolFile("invalid/oversized.json"));
}

void test_rejects_missing_station() {
  assertRejected(readProtocolFile("invalid/missing-station.json"));
}

void test_rejects_wrong_type() { assertRejected(readProtocolFile("invalid/wrong-type.json")); }

void test_rejects_document_that_is_not_an_object() { assertRejected(R"(["de:09162:2"])"); }

void test_encoded_settings_match_settings_read_example() {
  Settings settings = decodeOrFail(readProtocolFile("valid/settings-write.json"), storedSettings());
  std::string encoded = encodeSettings(settings);
  TEST_ASSERT_TRUE(sameJson(encoded, readProtocolFile("valid/settings-read.json")));
  TEST_ASSERT_EQUAL(std::string::npos, encoded.find("wifiPassword"));
}

void test_password_included_on_request_round_trips() {
  Settings settings = decodeOrFail(readProtocolFile("valid/settings-write.json"), storedSettings());
  Settings decoded = decodeOrFail(encodeSettings(settings, Password::Include), storedSettings());
  TEST_ASSERT_EQUAL_STRING("correct horse", decoded.wifiPassword.c_str());
  TEST_ASSERT_EQUAL_STRING("home", decoded.wifiSsid.c_str());
  TEST_ASSERT_EQUAL(2, decoded.boardSelection.routeDirectionIds.size());
}

void test_encodes_info() {
  TEST_ASSERT_TRUE(sameJson(encodeInfo("mvv", "v0.1.0"), readProtocolFile("valid/info.json")));
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_decodes_settings_write_with_password);
  RUN_TEST(test_missing_password_keeps_stored_password);
  RUN_TEST(test_empty_password_keeps_stored_password);
  RUN_TEST(test_rejects_malformed_json);
  RUN_TEST(test_rejects_oversized_document);
  RUN_TEST(test_rejects_missing_station);
  RUN_TEST(test_rejects_wrong_type);
  RUN_TEST(test_rejects_document_that_is_not_an_object);
  RUN_TEST(test_encoded_settings_match_settings_read_example);
  RUN_TEST(test_password_included_on_request_round_trips);
  RUN_TEST(test_encodes_info);
  return UNITY_END();
}
