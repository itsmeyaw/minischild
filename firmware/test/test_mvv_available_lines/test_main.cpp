#include <unity.h>

#include <cstring>
#include <string>
#include <vector>

#include "fixture.h"
#include "provider/mvv/available_lines_parser.h"
#include "provider/mvv/departures_url.h"

void setUp() {}
void tearDown() {}

static std::vector<std::string> idsFrom(const RouteDirectionsResult& result) {
  const std::vector<std::string>* ids = std::get_if<std::vector<std::string>>(&result);
  if (!ids) {
    TEST_FAIL_MESSAGE("expected route direction IDs");
    return {};
  }
  return *ids;
}

void test_parses_all_route_direction_ids_in_order() {
  std::vector<std::string> ids = idsFrom(parseMvvAvailableLines(kMvvAvailableLinesFixture));
  TEST_ASSERT_EQUAL(8, ids.size());
  TEST_ASSERT_EQUAL_STRING("ddb:920S1: :H:j26", ids[0].c_str());
  TEST_ASSERT_EQUAL_STRING("swm:010U5:G:H:016", ids[1].c_str());
  TEST_ASSERT_EQUAL_STRING("swm:02021:G:H:016", ids[2].c_str());
  TEST_ASSERT_EQUAL_STRING("swm:32919:G:H:016", ids[3].c_str());
  TEST_ASSERT_EQUAL_STRING("swm:03054:G:H:016", ids[4].c_str());
  TEST_ASSERT_EQUAL_STRING("swm:03100:G:R:016", ids[5].c_str());
  TEST_ASSERT_EQUAL_STRING("swm:33N43:G:H:016", ids[6].c_str());
  TEST_ASSERT_EQUAL_STRING("swm:20X30:G:H:016", ids[7].c_str());
}

void test_unknown_station_gives_no_route_directions() {
  std::vector<std::string> ids =
      idsFrom(parseMvvAvailableLines(R"({"error":"","lines":[],"icons":[]})"));
  TEST_ASSERT_TRUE(ids.empty());
}

void test_error_field_is_a_fetch_error() {
  RouteDirectionsResult result = parseMvvAvailableLines(R"({"error":"Haltestelle unbekannt"})");
  const FetchError* error = std::get_if<FetchError>(&result);
  TEST_ASSERT_NOT_NULL(error);
  TEST_ASSERT_NOT_NULL(strstr(error->message.c_str(), "Haltestelle unbekannt"));
}

void test_malformed_json_is_a_fetch_error() {
  RouteDirectionsResult result = parseMvvAvailableLines(std::string(R"({"error":)"));
  TEST_ASSERT_NOT_NULL(std::get_if<FetchError>(&result));
}

void test_builds_the_departures_url_from_parsed_route_directions() {
  std::vector<std::string> ids = idsFrom(parseMvvAvailableLines(
      R"({"error":"","lines":[{"stateless":"ddb:920S1: :H:j26"},{"stateless":"swm:010U3:G:H:016"}]})"));
  // 2026-10-19 12:00:00 UTC
  TEST_ASSERT_EQUAL_STRING(
      "https://www.mvv-muenchen.de/"
      "?eID=departuresFinder&action=get_departures&stop_id=de%3A09162%3A2"
      "&requested_timestamp=1792411200&lines="
      "JmxpbmU9ZGRiJTNBOTIwUzElM0ElMjAlM0FIJTNBajI2JmxpbmU9c3dtJTNBMDEwVTMlM0FHJTNBSCUzQTAxNg%3D%"
      "3D",
      mvvDeparturesUrl("de:09162:2", ids, 1792411200).c_str());
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_parses_all_route_direction_ids_in_order);
  RUN_TEST(test_unknown_station_gives_no_route_directions);
  RUN_TEST(test_error_field_is_a_fetch_error);
  RUN_TEST(test_malformed_json_is_a_fetch_error);
  RUN_TEST(test_builds_the_departures_url_from_parsed_route_directions);
  return UNITY_END();
}
