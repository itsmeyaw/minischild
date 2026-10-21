#include <unity.h>

#include <cstring>
#include <ctime>
#include <string>
#include <vector>

#include "fixture.h"
#include "model/departure_board.h"
#include "provider/bvg/departure_parser.h"

// 2026-10-21 18:15:08 CEST, when the fixture was captured
static const time_t NOW = 1792599308;

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

void test_maps_route_headsign_platform_and_station() {
  DepartureBoard board = boardFrom(parseBvgDepartures(kBvgFixture, NOW, {}));
  TEST_ASSERT_EQUAL_STRING("S+U Alexanderplatz Bhf (Berlin)", board.stopName.c_str());
  TEST_ASSERT_EQUAL(NOW, board.timestamp);
  const Departure& d = board.departures[0];
  TEST_ASSERT_EQUAL_STRING("U5", d.routeShortName.c_str());
  TEST_ASSERT_EQUAL_STRING("Hauptbahnhof", d.tripHeadsign.c_str());
  TEST_ASSERT_EQUAL_STRING("1 (U5)", d.platformCode.c_str());
}

void test_on_time_departure_is_scheduled_with_zero_delay() {
  DepartureBoard board = boardFrom(parseBvgDepartures(kBvgFixture, NOW, {}));
  const Departure& d = board.departures[0];
  TEST_ASSERT_EQUAL(1792599120, d.scheduledDepartureTime);
  TEST_ASSERT_EQUAL(ScheduleRelationship::Scheduled, d.scheduleRelationship);
  TEST_ASSERT_TRUE(d.departure.has_value());
  TEST_ASSERT_EQUAL(1792599120, d.departure->time);
  TEST_ASSERT_EQUAL(0, d.departure->delay);
}

void test_delayed_departure_has_predicted_time_and_positive_delay() {
  DepartureBoard board = boardFrom(parseBvgDepartures(kBvgFixture, NOW, {}));
  const Departure& d = board.departures[1];
  TEST_ASSERT_EQUAL(1792599300, d.scheduledDepartureTime);
  TEST_ASSERT_TRUE(d.departure.has_value());
  TEST_ASSERT_EQUAL(1792599420, d.departure->time);
  TEST_ASSERT_EQUAL(120, d.departure->delay);
}

void test_early_departure_has_negative_delay() {
  DepartureBoard board = boardFrom(parseBvgDepartures(kBvgFixture, NOW, {}));
  const Departure& d = board.departures[2];
  TEST_ASSERT_TRUE(d.departure.has_value());
  TEST_ASSERT_EQUAL(1792599240, d.departure->time);
  TEST_ASSERT_EQUAL(-60, d.departure->delay);
}

void test_null_delay_is_no_data_without_event() {
  DepartureBoard board = boardFrom(parseBvgDepartures(kBvgFixture, NOW, {}));
  const Departure& d = board.departures[3];
  TEST_ASSERT_EQUAL(1792599360, d.scheduledDepartureTime);
  TEST_ASSERT_EQUAL(ScheduleRelationship::NoData, d.scheduleRelationship);
  TEST_ASSERT_FALSE(d.departure.has_value());
}

void test_null_platform_falls_back_to_planned_platform() {
  DepartureBoard board = boardFrom(parseBvgDepartures(kBvgFixture, NOW, {}));
  TEST_ASSERT_EQUAL_STRING("2", board.departures[4].platformCode.c_str());
}

void test_cancelled_departures_are_dropped() {
  DepartureBoard board = boardFrom(parseBvgDepartures(kBvgFixture, NOW, {}));
  TEST_ASSERT_EQUAL(6, board.departures.size());
  TEST_ASSERT_EQUAL(1792600200, board.departures[5].scheduledDepartureTime);
}

void test_departure_with_unparseable_planned_time_is_dropped() {
  DepartureBoard board = boardFrom(parseBvgDepartures(
      R"({"departures":[{"line":{"name":"U5"},"direction":"Hauptbahnhof","plannedWhen":"soon"},)"
      R"({"line":{"name":"U5"},"direction":"Hauptbahnhof","plannedWhen":"2026-10-21T18:20:00+02:00"}]})",
      NOW, {}));
  TEST_ASSERT_EQUAL(1, board.departures.size());
  TEST_ASSERT_EQUAL(1792599600, board.departures[0].scheduledDepartureTime);
}

void test_malformed_json_is_a_fetch_error() {
  FetchResult result = parseBvgDepartures(std::string(R"({"departures":)"), NOW, {});
  const FetchError* error = std::get_if<FetchError>(&result);
  TEST_ASSERT_NOT_NULL(error);
  TEST_ASSERT_NOT_NULL(strstr(error->message.c_str(), "BVG JSON"));
}

void test_message_field_is_a_fetch_error() {
  FetchResult result = parseBvgDepartures(R"({"message":"Haltestelle nicht gefunden"})", NOW, {});
  const FetchError* error = std::get_if<FetchError>(&result);
  TEST_ASSERT_NOT_NULL(error);
  TEST_ASSERT_EQUAL_STRING("BVG: Haltestelle nicht gefunden", error->message.c_str());
}

void test_no_departures_gives_an_empty_board() {
  DepartureBoard board = boardFrom(parseBvgDepartures(R"({"departures":[]})", NOW, {}));
  TEST_ASSERT_EQUAL_STRING("", board.stopName.c_str());
  TEST_ASSERT_TRUE(board.departures.empty());
}

void test_missing_departures_is_a_fetch_error() {
  FetchResult result = parseBvgDepartures(R"({})", NOW, {});
  const FetchError* error = std::get_if<FetchError>(&result);
  TEST_ASSERT_NOT_NULL(error);
  TEST_ASSERT_EQUAL_STRING("BVG: no departures", error->message.c_str());
}

void test_empty_route_directions_keep_every_departure() {
  DepartureBoard board = boardFrom(parseBvgDepartures(kBvgFixture, NOW, {}));
  TEST_ASSERT_EQUAL(6, board.departures.size());
}

void test_keeps_only_departures_of_the_chosen_route_direction() {
  DepartureBoard board = boardFrom(parseBvgDepartures(kBvgFixture, NOW, {"M6|900100038"}));
  TEST_ASSERT_EQUAL(1, board.departures.size());
  TEST_ASSERT_EQUAL_STRING("Mitte, Am Kupfergraben", board.departures[0].tripHeadsign.c_str());
}

void test_keeps_departures_of_every_chosen_route_direction() {
  DepartureBoard board =
      boardFrom(parseBvgDepartures(kBvgFixture, NOW, {"M6|900100038", "U5|900003201"}));
  TEST_ASSERT_EQUAL(2, board.departures.size());
  TEST_ASSERT_EQUAL_STRING("U5", board.departures[0].routeShortName.c_str());
  TEST_ASSERT_EQUAL_STRING("M6", board.departures[1].routeShortName.c_str());
}

void test_departure_without_destination_matches_the_empty_destination_id() {
  DepartureBoard board = boardFrom(parseBvgDepartures(kBvgFixture, NOW, {"U5|"}));
  TEST_ASSERT_EQUAL(1, board.departures.size());
  TEST_ASSERT_EQUAL_STRING("2", board.departures[0].platformCode.c_str());
}

void test_route_direction_match_is_exact() {
  DepartureBoard board = boardFrom(parseBvgDepartures(kBvgFixture, NOW, {"M6"}));
  TEST_ASSERT_TRUE(board.departures.empty());
}

void test_stop_name_is_kept_when_the_first_departure_is_cancelled() {
  DepartureBoard board = boardFrom(parseBvgDepartures(
      R"({"departures":[{"cancelled":true,"stop":{"name":"Stop A"},"line":{"name":"U5"},)"
      R"("plannedWhen":"2026-10-21T18:20:00+02:00"},)"
      R"({"stop":{"name":"Stop A"},"line":{"name":"U5"},)"
      R"("plannedWhen":"2026-10-21T18:25:00+02:00"}]})",
      NOW, {}));
  TEST_ASSERT_EQUAL_STRING("Stop A", board.stopName.c_str());
  TEST_ASSERT_EQUAL(1, board.departures.size());
}

void test_delay_without_when_is_no_data() {
  DepartureBoard board = boardFrom(parseBvgDepartures(
      R"({"departures":[{"stop":{"name":"Stop A"},"line":{"name":"U5"},"delay":60,"when":null,)"
      R"("plannedWhen":"2026-10-21T18:20:00+02:00"}]})",
      NOW, {}));
  TEST_ASSERT_EQUAL(1, board.departures.size());
  const Departure& d = board.departures[0];
  TEST_ASSERT_EQUAL(ScheduleRelationship::NoData, d.scheduleRelationship);
  TEST_ASSERT_FALSE(d.departure.has_value());
}

void test_null_line_has_an_empty_route_short_name() {
  DepartureBoard board =
      boardFrom(parseBvgDepartures(R"({"departures":[{"line":null,"direction":"Hauptbahnhof",)"
                                   R"("plannedWhen":"2026-10-21T18:20:00+02:00"}]})",
                                   NOW, {}));
  TEST_ASSERT_EQUAL(1, board.departures.size());
  TEST_ASSERT_EQUAL_STRING("", board.departures[0].routeShortName.c_str());
}

void test_null_line_is_excluded_by_a_route_direction_filter() {
  DepartureBoard board =
      boardFrom(parseBvgDepartures(R"({"departures":[{"line":null,"direction":"Hauptbahnhof",)"
                                   R"("plannedWhen":"2026-10-21T18:20:00+02:00"}]})",
                                   NOW, {"U5|"}));
  TEST_ASSERT_TRUE(board.departures.empty());
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_maps_route_headsign_platform_and_station);
  RUN_TEST(test_on_time_departure_is_scheduled_with_zero_delay);
  RUN_TEST(test_delayed_departure_has_predicted_time_and_positive_delay);
  RUN_TEST(test_early_departure_has_negative_delay);
  RUN_TEST(test_null_delay_is_no_data_without_event);
  RUN_TEST(test_null_platform_falls_back_to_planned_platform);
  RUN_TEST(test_cancelled_departures_are_dropped);
  RUN_TEST(test_departure_with_unparseable_planned_time_is_dropped);
  RUN_TEST(test_malformed_json_is_a_fetch_error);
  RUN_TEST(test_message_field_is_a_fetch_error);
  RUN_TEST(test_no_departures_gives_an_empty_board);
  RUN_TEST(test_missing_departures_is_a_fetch_error);
  RUN_TEST(test_empty_route_directions_keep_every_departure);
  RUN_TEST(test_keeps_only_departures_of_the_chosen_route_direction);
  RUN_TEST(test_keeps_departures_of_every_chosen_route_direction);
  RUN_TEST(test_departure_without_destination_matches_the_empty_destination_id);
  RUN_TEST(test_route_direction_match_is_exact);
  RUN_TEST(test_stop_name_is_kept_when_the_first_departure_is_cancelled);
  RUN_TEST(test_delay_without_when_is_no_data);
  RUN_TEST(test_null_line_has_an_empty_route_short_name);
  RUN_TEST(test_null_line_is_excluded_by_a_route_direction_filter);
  return UNITY_END();
}
