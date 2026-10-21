#include <unity.h>

#include "board/board_selection.h"

// 2026-10-19 12:00:00 UTC
static const time_t NOON = 1792411200;

static Departure scheduledAt(const char* route, time_t t) {
  return Departure{route, "Freising", "1", t, ScheduleRelationship::Scheduled, std::nullopt};
}

void setUp() {}
void tearDown() {}

void test_sorts_by_predicted_time_when_present_else_scheduled() {
  Departure a = scheduledAt("A", NOON + 5 * 60);
  a.departure = StopTimeEvent{NOON + 9 * 60, 4 * 60};
  Departure b = scheduledAt("B", NOON + 7 * 60);
  std::vector<Departure> sorted = selectDepartures({a, b}, {}, NOON, 5);
  TEST_ASSERT_EQUAL(2, sorted.size());
  TEST_ASSERT_EQUAL_STRING("B", sorted[0].routeShortName.c_str());
  TEST_ASSERT_EQUAL_STRING("A", sorted[1].routeShortName.c_str());
}

void test_drops_departures_before_the_current_minute() {
  std::vector<Departure> sorted = selectDepartures({scheduledAt("P", NOON - 2 * 60)}, {}, NOON, 5);
  TEST_ASSERT_EQUAL(0, sorted.size());
}

void test_keeps_departure_within_the_current_minute() {
  std::vector<Departure> sorted = selectDepartures({scheduledAt("C", NOON)}, {}, NOON + 30, 5);
  TEST_ASSERT_EQUAL(1, sorted.size());
  TEST_ASSERT_EQUAL_STRING("C", sorted[0].routeShortName.c_str());
}

void test_keeps_departure_predicted_in_the_future_despite_past_schedule() {
  Departure d = scheduledAt("F", NOON - 2 * 60);
  d.departure = StopTimeEvent{NOON + 3 * 60, 5 * 60};
  std::vector<Departure> sorted = selectDepartures({d}, {}, NOON, 5);
  TEST_ASSERT_EQUAL(1, sorted.size());
  TEST_ASSERT_EQUAL_STRING("F", sorted[0].routeShortName.c_str());
}

void test_keeps_only_the_first_limit_departures() {
  std::vector<Departure> departures;
  for (int i = 0; i < 7; i++) {
    departures.push_back(scheduledAt(std::to_string(i).c_str(), NOON + (i + 1) * 60));
  }
  std::vector<Departure> sorted = selectDepartures(departures, {}, NOON, 3);
  TEST_ASSERT_EQUAL(3, sorted.size());
  TEST_ASSERT_EQUAL_STRING("0", sorted[0].routeShortName.c_str());
  TEST_ASSERT_EQUAL_STRING("2", sorted[2].routeShortName.c_str());
}

void test_equal_times_keep_input_order() {
  std::vector<Departure> sorted =
      selectDepartures({scheduledAt("X", NOON + 60), scheduledAt("Y", NOON + 60)}, {}, NOON, 5);
  TEST_ASSERT_EQUAL(2, sorted.size());
  TEST_ASSERT_EQUAL_STRING("X", sorted[0].routeShortName.c_str());
  TEST_ASSERT_EQUAL_STRING("Y", sorted[1].routeShortName.c_str());
}

void test_keeps_only_departures_on_listed_platforms() {
  Departure a = scheduledAt("A", NOON + 60);
  Departure b = scheduledAt("B", NOON + 60);
  b.platformCode = "10";
  Departure c = scheduledAt("C", NOON + 60);
  c.platformCode = "U-Bahn Gleis 1";
  std::vector<Departure> sorted = selectDepartures({a, b, c}, {"1"}, NOON, 5);
  TEST_ASSERT_EQUAL(1, sorted.size());
  TEST_ASSERT_EQUAL_STRING("A", sorted[0].routeShortName.c_str());
}

void test_empty_platform_list_keeps_departures_on_every_platform() {
  Departure a = scheduledAt("A", NOON + 60);
  Departure b = scheduledAt("B", NOON + 60);
  b.platformCode = "";
  Departure c = scheduledAt("C", NOON + 60);
  c.platformCode = "U-Bahn Gleis 1";
  std::vector<Departure> sorted = selectDepartures({a, b, c}, {}, NOON, 5);
  TEST_ASSERT_EQUAL(3, sorted.size());
}

void test_departure_without_platform_is_hidden_when_platforms_are_listed() {
  Departure a = scheduledAt("A", NOON + 60);
  a.platformCode = "";
  std::vector<Departure> sorted = selectDepartures({a}, {"1"}, NOON, 5);
  TEST_ASSERT_EQUAL(0, sorted.size());
}

void test_platform_filter_applies_before_the_limit() {
  Departure a = scheduledAt("A", NOON + 1 * 60);
  a.platformCode = "2";
  Departure b = scheduledAt("B", NOON + 2 * 60);
  b.platformCode = "2";
  Departure c = scheduledAt("C", NOON + 3 * 60);
  Departure d = scheduledAt("D", NOON + 4 * 60);
  std::vector<Departure> sorted = selectDepartures({a, b, c, d}, {"1"}, NOON, 2);
  TEST_ASSERT_EQUAL(2, sorted.size());
  TEST_ASSERT_EQUAL_STRING("C", sorted[0].routeShortName.c_str());
  TEST_ASSERT_EQUAL_STRING("D", sorted[1].routeShortName.c_str());
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_sorts_by_predicted_time_when_present_else_scheduled);
  RUN_TEST(test_drops_departures_before_the_current_minute);
  RUN_TEST(test_keeps_departure_within_the_current_minute);
  RUN_TEST(test_keeps_departure_predicted_in_the_future_despite_past_schedule);
  RUN_TEST(test_keeps_only_the_first_limit_departures);
  RUN_TEST(test_equal_times_keep_input_order);
  RUN_TEST(test_keeps_only_departures_on_listed_platforms);
  RUN_TEST(test_empty_platform_list_keeps_departures_on_every_platform);
  RUN_TEST(test_departure_without_platform_is_hidden_when_platforms_are_listed);
  RUN_TEST(test_platform_filter_applies_before_the_limit);
  return UNITY_END();
}
