#include <unity.h>

#include <cstdlib>
#include <cstring>
#include <ctime>
#include <string>

#include "fixture.h"
#include "model/departure_board.h"
#include "provider/mvv/departure_parser.h"

// 2026-10-19 23:29:21 CEST, when the fixture was captured
static const time_t NOW = 1792445361;

static DepartureBoard boardFrom(const FetchResult& result) {
  const DepartureBoard* board = std::get_if<DepartureBoard>(&result);
  if (!board) {
    TEST_FAIL_MESSAGE("expected a DepartureBoard");
    return DepartureBoard{};
  }
  return *board;
}

void setUp() {
  setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 1);
  tzset();
}
void tearDown() {}

void test_maps_route_headsign_platform_and_station() {
  DepartureBoard board = boardFrom(parseMvvDepartures(kMvvFixture, NOW));
  TEST_ASSERT_EQUAL_STRING("Marienplatz", board.stopName.c_str());
  TEST_ASSERT_EQUAL(NOW, board.timestamp);
  TEST_ASSERT_EQUAL(5, board.departures.size());
  const Departure& d = board.departures[0];
  TEST_ASSERT_EQUAL_STRING("U6", d.routeShortName.c_str());
  TEST_ASSERT_EQUAL_STRING("Fröttmaning", d.tripHeadsign.c_str());
  TEST_ASSERT_EQUAL_STRING("U-Bahn Gleis 1", d.platformCode.c_str());
}

void test_on_time_departure_has_zero_delay() {
  DepartureBoard board = boardFrom(parseMvvDepartures(kMvvFixture, NOW));
  const Departure& d = board.departures[0];
  TEST_ASSERT_EQUAL(ScheduleRelationship::Scheduled, d.scheduleRelationship);
  TEST_ASSERT_TRUE(d.departure.has_value());
  TEST_ASSERT_EQUAL(1792445580, d.departure->time);
  TEST_ASSERT_EQUAL(0, d.departure->delay);
}

void test_delayed_departure_has_predicted_time_and_positive_delay() {
  DepartureBoard board = boardFrom(parseMvvDepartures(kMvvFixture, NOW));
  const Departure& d = board.departures[1];
  TEST_ASSERT_TRUE(d.departure.has_value());
  TEST_ASSERT_EQUAL(1792446060, d.departure->time);
  TEST_ASSERT_EQUAL(180, d.departure->delay);
}

void test_departure_after_midnight_uses_departure_date() {
  DepartureBoard board = boardFrom(parseMvvDepartures(kMvvFixture, NOW));
  TEST_ASSERT_EQUAL(1792447380, board.departures[3].scheduledDepartureTime);
}

void test_delay_across_midnight_lands_on_next_day() {
  DepartureBoard board = boardFrom(parseMvvDepartures(kMvvFixture, NOW));
  const Departure& d = board.departures[2];
  TEST_ASSERT_EQUAL(1792447140, d.scheduledDepartureTime);
  TEST_ASSERT_TRUE(d.departure.has_value());
  TEST_ASSERT_EQUAL(1792447380, d.departure->time);
  TEST_ASSERT_EQUAL(240, d.departure->delay);
}

void test_early_departure_across_midnight_lands_on_previous_day() {
  DepartureBoard board = boardFrom(parseMvvDepartures(
      R"({"error":"","departures":[{"line":{"number":"S1"},"direction":"Freising",)"
      R"("station":{"name":"Marienplatz"},"track":"2","departureDate":"20261020",)"
      R"("departurePlanned":"00:03","departureLive":"23:58"}]})",
      NOW));
  const Departure& d = board.departures[0];
  TEST_ASSERT_EQUAL(1792447380, d.scheduledDepartureTime);
  TEST_ASSERT_EQUAL(1792447080, d.departure->time);
  TEST_ASSERT_EQUAL(-300, d.departure->delay);
}

void test_empty_live_time_is_no_data_without_event() {
  DepartureBoard board = boardFrom(parseMvvDepartures(kMvvFixture, NOW));
  const Departure& d = board.departures[4];
  TEST_ASSERT_EQUAL(ScheduleRelationship::NoData, d.scheduleRelationship);
  TEST_ASSERT_FALSE(d.departure.has_value());
  TEST_ASSERT_EQUAL(1792448280, d.scheduledDepartureTime);
}

void test_canceled_departure_is_skipped_without_event() {
  DepartureBoard board = boardFrom(parseMvvDepartures(
      R"J({"error":"","departures":[{"line":{"number":"28"},"direction":"Sendlinger Tor",)J"
      R"J("station":{"name":"Karlsplatz (Stachus)"},"track":"Pos. 4","departureDate":"20261022",)J"
      R"J("departurePlanned":"13:54","departureLive":"Halt entf\u00e4llt","inTime":false}]})J",
      NOW));
  const Departure& d = board.departures[0];
  TEST_ASSERT_EQUAL(ScheduleRelationship::Skipped, d.scheduleRelationship);
  TEST_ASSERT_FALSE(d.departure.has_value());
  TEST_ASSERT_EQUAL(1792670040, d.scheduledDepartureTime);
}

void test_error_field_is_a_fetch_error() {
  FetchResult result =
      parseMvvDepartures(R"({"error":"Haltestelle unbekannt","departures":[]})", NOW);
  const FetchError* error = std::get_if<FetchError>(&result);
  TEST_ASSERT_NOT_NULL(error);
  TEST_ASSERT_NOT_NULL(strstr(error->message.c_str(), "Haltestelle unbekannt"));
}

void test_malformed_json_is_a_fetch_error() {
  FetchResult result = parseMvvDepartures(std::string(R"({"error":)"), NOW);
  TEST_ASSERT_NOT_NULL(std::get_if<FetchError>(&result));
}

void test_no_departures_gives_an_empty_board() {
  DepartureBoard board = boardFrom(parseMvvDepartures(R"({"error":"","departures":[]})", NOW));
  TEST_ASSERT_EQUAL_STRING("", board.stopName.c_str());
  TEST_ASSERT_TRUE(board.departures.empty());
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_maps_route_headsign_platform_and_station);
  RUN_TEST(test_on_time_departure_has_zero_delay);
  RUN_TEST(test_delayed_departure_has_predicted_time_and_positive_delay);
  RUN_TEST(test_departure_after_midnight_uses_departure_date);
  RUN_TEST(test_delay_across_midnight_lands_on_next_day);
  RUN_TEST(test_early_departure_across_midnight_lands_on_previous_day);
  RUN_TEST(test_empty_live_time_is_no_data_without_event);
  RUN_TEST(test_canceled_departure_is_skipped_without_event);
  RUN_TEST(test_error_field_is_a_fetch_error);
  RUN_TEST(test_malformed_json_is_a_fetch_error);
  RUN_TEST(test_no_departures_gives_an_empty_board);
  return UNITY_END();
}
