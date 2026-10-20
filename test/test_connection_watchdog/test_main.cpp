#include <unity.h>

#include <climits>

#include "board/connection_watchdog.h"

void setUp() {}

void tearDown() {}

void test_connected_never_asks_for_recovery() {
  ConnectionWatchdog watchdog;

  TEST_ASSERT_EQUAL(WatchdogAction::None, watchdog.check(true, 0));
  TEST_ASSERT_EQUAL(WatchdogAction::None, watchdog.check(true, 60UL * 60 * 1000));
}

void test_reconnects_every_thirty_seconds_while_down() {
  ConnectionWatchdog watchdog;
  watchdog.check(false, 0);

  TEST_ASSERT_EQUAL(WatchdogAction::None, watchdog.check(false, 29UL * 1000));
  TEST_ASSERT_EQUAL(WatchdogAction::Reconnect, watchdog.check(false, 30UL * 1000));
  TEST_ASSERT_EQUAL(WatchdogAction::None, watchdog.check(false, 45UL * 1000));
}

void test_restarts_after_ten_minutes_down() {
  ConnectionWatchdog watchdog;
  watchdog.check(false, 0);

  TEST_ASSERT_EQUAL(WatchdogAction::Restart, watchdog.check(false, 10UL * 60 * 1000));
}

void test_reconnecting_resets_the_restart_timer() {
  ConnectionWatchdog watchdog;
  watchdog.check(false, 0);
  watchdog.check(true, 60UL * 1000);

  watchdog.check(false, 5UL * 60 * 1000);

  TEST_ASSERT_NOT_EQUAL(WatchdogAction::Restart, watchdog.check(false, 14UL * 60 * 1000));
  TEST_ASSERT_EQUAL(WatchdogAction::Restart, watchdog.check(false, 15UL * 60 * 1000));
}

void test_outage_timing_survives_millis_wraparound() {
  const unsigned long start = ULONG_MAX - 1000;
  ConnectionWatchdog watchdog;
  watchdog.check(false, start);

  TEST_ASSERT_EQUAL(WatchdogAction::Reconnect, watchdog.check(false, start + 30UL * 1000));
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_connected_never_asks_for_recovery);
  RUN_TEST(test_reconnects_every_thirty_seconds_while_down);
  RUN_TEST(test_restarts_after_ten_minutes_down);
  RUN_TEST(test_reconnecting_resets_the_restart_timer);
  RUN_TEST(test_outage_timing_survives_millis_wraparound);
  return UNITY_END();
}
