#include <unity.h>

#include <cstring>
#include <ctime>
#include <string>
#include <vector>

#include "fixture.h"
#include "model/departure_board.h"
#include "provider/efa/departure_parser.h"

// 2026-10-21 15:55:00 UTC, when the fixture was captured
static const time_t NOW = 1792598100;

static DepartureBoard boardFrom(const FetchResult& result) {
  const DepartureBoard* board = std::get_if<DepartureBoard>(&result);
  if (!board) {
    TEST_FAIL_MESSAGE("expected a DepartureBoard");
    return DepartureBoard{};
  }
  return *board;
}

void setUp() {}
void tearDown() {}

void test_stop_name_comes_from_the_disassembled_name_of_the_requested_stop() {
  DepartureBoard board = boardFrom(parseEfaDepartures(kEfaFixture, NOW, {}));
  TEST_ASSERT_EQUAL_STRING("Hauptbahnhof (tief)", board.stopName.c_str());
  TEST_ASSERT_EQUAL(NOW, board.timestamp);
}

void test_maps_route_headsign_platform_and_scheduled_time() {
  DepartureBoard board = boardFrom(parseEfaDepartures(kEfaFixture, NOW, {}));
  const Departure& d = board.departures[0];
  TEST_ASSERT_EQUAL_STRING("S60", d.routeShortName.c_str());
  TEST_ASSERT_EQUAL_STRING("Stuttgart Flughafen/Messe", d.tripHeadsign.c_str());
  TEST_ASSERT_EQUAL_STRING("Gleis 101", d.platformCode.c_str());
  TEST_ASSERT_EQUAL(1792598280, d.scheduledDepartureTime);
}

void test_on_time_departure_is_scheduled_with_zero_delay() {
  DepartureBoard board = boardFrom(parseEfaDepartures(kEfaFixture, NOW, {}));
  const Departure& d = board.departures[0];
  TEST_ASSERT_EQUAL(ScheduleRelationship::Scheduled, d.scheduleRelationship);
  TEST_ASSERT_TRUE(d.departure.has_value());
  TEST_ASSERT_EQUAL(1792598280, d.departure->time);
  TEST_ASSERT_EQUAL(0, d.departure->delay);
}

void test_delayed_departure_has_predicted_time_and_positive_delay() {
  DepartureBoard board = boardFrom(parseEfaDepartures(kEfaFixture, NOW, {}));
  const Departure& d = board.departures[1];
  TEST_ASSERT_EQUAL(1792598400, d.scheduledDepartureTime);
  TEST_ASSERT_TRUE(d.departure.has_value());
  TEST_ASSERT_EQUAL(1792598580, d.departure->time);
  TEST_ASSERT_EQUAL(180, d.departure->delay);
}

void test_early_departure_has_negative_delay() {
  DepartureBoard board = boardFrom(parseEfaDepartures(kEfaFixture, NOW, {}));
  const Departure& d = board.departures[2];
  TEST_ASSERT_TRUE(d.departure.has_value());
  TEST_ASSERT_EQUAL(1792598880, d.departure->time);
  TEST_ASSERT_EQUAL(-120, d.departure->delay);
}

void test_null_estimate_is_no_data_without_event() {
  DepartureBoard board = boardFrom(parseEfaDepartures(kEfaFixture, NOW, {}));
  const Departure& d = board.departures[3];
  TEST_ASSERT_EQUAL(1792599300, d.scheduledDepartureTime);
  TEST_ASSERT_EQUAL(ScheduleRelationship::NoData, d.scheduleRelationship);
  TEST_ASSERT_FALSE(d.departure.has_value());
}

void test_platform_falls_back_to_the_platform_property() {
  DepartureBoard board = boardFrom(parseEfaDepartures(kEfaFixture, NOW, {}));
  TEST_ASSERT_EQUAL_STRING("2", board.departures[1].platformCode.c_str());
}

void test_bus_without_disassembled_name_uses_the_number_as_route_name() {
  DepartureBoard board = boardFrom(parseEfaDepartures(kEfaFixture, NOW, {}));
  TEST_ASSERT_EQUAL_STRING("127", board.departures[4].routeShortName.c_str());
}

void test_cancelled_departures_are_dropped() {
  DepartureBoard board = boardFrom(parseEfaDepartures(kEfaFixture, NOW, {}));
  TEST_ASSERT_EQUAL(6, board.departures.size());
  for (const Departure& d : board.departures) {
    TEST_ASSERT_NOT_EQUAL(1792599600, d.scheduledDepartureTime);
  }
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_stop_name_comes_from_the_disassembled_name_of_the_requested_stop);
  RUN_TEST(test_maps_route_headsign_platform_and_scheduled_time);
  RUN_TEST(test_on_time_departure_is_scheduled_with_zero_delay);
  RUN_TEST(test_delayed_departure_has_predicted_time_and_positive_delay);
  RUN_TEST(test_early_departure_has_negative_delay);
  RUN_TEST(test_null_estimate_is_no_data_without_event);
  RUN_TEST(test_platform_falls_back_to_the_platform_property);
  RUN_TEST(test_bus_without_disassembled_name_uses_the_number_as_route_name);
  RUN_TEST(test_cancelled_departures_are_dropped);
  return UNITY_END();
}
