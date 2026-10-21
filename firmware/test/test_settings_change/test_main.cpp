#include <unity.h>

#include "settings/settings.h"

static Settings running() {
  return Settings{{"de:09162:2", {"ddb:920S1: :H:j26"}, {"1"}}, "home", "secret"};
}

void setUp() {}
void tearDown() {}

void test_same_settings_change_nothing() {
  TEST_ASSERT_TRUE(settingsChange(running(), running()) == SettingsChange::None);
}

void test_new_station_changes_the_board_selection() {
  Settings saved = running();
  saved.boardSelection.stationId = "de:09162:6";
  TEST_ASSERT_TRUE(settingsChange(running(), saved) == SettingsChange::BoardSelection);
}

void test_new_route_directions_change_the_board_selection() {
  Settings saved = running();
  saved.boardSelection.routeDirectionIds = {};
  TEST_ASSERT_TRUE(settingsChange(running(), saved) == SettingsChange::BoardSelection);
}

void test_new_platforms_change_the_board_selection() {
  Settings saved = running();
  saved.boardSelection.platforms = {"2"};
  TEST_ASSERT_TRUE(settingsChange(running(), saved) == SettingsChange::BoardSelection);
}

void test_new_ssid_changes_the_wifi() {
  Settings saved = running();
  saved.wifiSsid = "office";
  TEST_ASSERT_TRUE(settingsChange(running(), saved) == SettingsChange::Wifi);
}

void test_new_password_changes_the_wifi() {
  Settings saved = running();
  saved.wifiPassword = "other";
  TEST_ASSERT_TRUE(settingsChange(running(), saved) == SettingsChange::Wifi);
}

void test_wifi_wins_over_a_board_selection_change() {
  Settings saved = running();
  saved.wifiSsid = "office";
  saved.boardSelection.stationId = "de:09162:6";
  TEST_ASSERT_TRUE(settingsChange(running(), saved) == SettingsChange::Wifi);
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_same_settings_change_nothing);
  RUN_TEST(test_new_station_changes_the_board_selection);
  RUN_TEST(test_new_route_directions_change_the_board_selection);
  RUN_TEST(test_new_platforms_change_the_board_selection);
  RUN_TEST(test_new_ssid_changes_the_wifi);
  RUN_TEST(test_new_password_changes_the_wifi);
  RUN_TEST(test_wifi_wins_over_a_board_selection_change);
  return UNITY_END();
}
