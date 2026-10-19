#include <unity.h>

#include "countdown.h"

// 2026-10-19 12:00:00 UTC
static const time_t NOON = 1792411200;

static Departure scheduledAt(time_t t) {
  return Departure{"S1", "Freising", "1", t, ScheduleRelationship::Scheduled, std::nullopt};
}

void setUp() {}
void tearDown() {}

void test_scheduled_departure_in_four_minutes() {
  TEST_ASSERT_EQUAL_STRING("4 min", countdownLabel(scheduledAt(NOON + 4 * 60), NOON).c_str());
}

void test_predicted_time_wins_over_scheduled() {
  Departure d = scheduledAt(NOON + 4 * 60);
  d.departure = StopTimeEvent{NOON + 7 * 60, 3 * 60};
  TEST_ASSERT_EQUAL_STRING("7 min", countdownLabel(d, NOON).c_str());
}

void test_now_is_truncated_to_the_minute() {
  TEST_ASSERT_EQUAL_STRING("4 min", countdownLabel(scheduledAt(NOON + 4 * 60), NOON + 50).c_str());
}

void test_departure_within_the_current_minute_is_now() {
  TEST_ASSERT_EQUAL_STRING("now", countdownLabel(scheduledAt(NOON + 30), NOON + 10).c_str());
}

void test_past_departure_is_now() {
  TEST_ASSERT_EQUAL_STRING("now", countdownLabel(scheduledAt(NOON - 2 * 60), NOON).c_str());
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_scheduled_departure_in_four_minutes);
  RUN_TEST(test_predicted_time_wins_over_scheduled);
  RUN_TEST(test_now_is_truncated_to_the_minute);
  RUN_TEST(test_departure_within_the_current_minute_is_now);
  RUN_TEST(test_past_departure_is_now);
  return UNITY_END();
}
