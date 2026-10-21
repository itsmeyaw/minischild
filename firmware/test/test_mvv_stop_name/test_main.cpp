#include <unity.h>

#include <cstring>
#include <string>

#include "fixture.h"
#include "provider/mvv/stop_name_parser.h"

void setUp() {}
void tearDown() {}

static std::string nameFrom(const StopNameResult& result) {
  const std::string* name = std::get_if<std::string>(&result);
  if (!name) {
    TEST_FAIL_MESSAGE("expected a stop name");
    return "";
  }
  return *name;
}

void test_returns_the_assigned_stop_name_of_the_station() {
  TEST_ASSERT_EQUAL_STRING("Marienplatz",
                           nameFrom(parseMvvStopName(kMvvStopFinderFixture, "de:09162:2")).c_str());
}

void test_returns_the_stop_name_of_a_station_in_a_locality() {
  TEST_ASSERT_EQUAL_STRING(
      "Martinsried, Ortsmitte",
      nameFrom(parseMvvStopName(kMvvStopFinderMartinsriedFixture, "de:09184:2544")).c_str());
}

void test_picks_the_location_whose_id_is_the_station() {
  StopNameResult result = parseMvvStopName(
      R"({"locations":[{"id":"de:09162:1","name":"Other","assignedStops":[{"id":"de:09162:1","name":"Other"}]},{"id":"de:09162:2","name":"München, Marienplatz","assignedStops":[{"id":"de:09162:2","name":"Marienplatz"}]}]})",
      "de:09162:2");
  TEST_ASSERT_EQUAL_STRING("Marienplatz", nameFrom(result).c_str());
}

void test_matches_the_assigned_stop_by_id_past_the_first() {
  StopNameResult result = parseMvvStopName(
      R"({"locations":[{"id":"de:09162:2","name":"München, Marienplatz","assignedStops":[{"id":"de:09162:9","name":"Other"},{"id":"de:09162:2","name":"Marienplatz"}]}]})",
      "de:09162:2");
  TEST_ASSERT_EQUAL_STRING("Marienplatz", nameFrom(result).c_str());
}

void test_falls_back_to_the_location_name_without_a_matching_assigned_stop() {
  StopNameResult result = parseMvvStopName(
      R"({"locations":[{"id":"de:09162:2","name":"München, Marienplatz","assignedStops":[{"id":"de:09162:9","name":"Other"}]}]})",
      "de:09162:2");
  TEST_ASSERT_EQUAL_STRING("München, Marienplatz", nameFrom(result).c_str());
}

void test_no_matching_location_is_a_fetch_error() {
  StopNameResult result = parseMvvStopName(kMvvStopFinderFixture, "de:09162:99");
  TEST_ASSERT_NOT_NULL(std::get_if<FetchError>(&result));
}

void test_malformed_json_is_a_fetch_error() {
  StopNameResult result = parseMvvStopName(std::string(R"({"locations":)"), "de:09162:2");
  TEST_ASSERT_NOT_NULL(std::get_if<FetchError>(&result));
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_returns_the_assigned_stop_name_of_the_station);
  RUN_TEST(test_returns_the_stop_name_of_a_station_in_a_locality);
  RUN_TEST(test_picks_the_location_whose_id_is_the_station);
  RUN_TEST(test_matches_the_assigned_stop_by_id_past_the_first);
  RUN_TEST(test_falls_back_to_the_location_name_without_a_matching_assigned_stop);
  RUN_TEST(test_no_matching_location_is_a_fetch_error);
  RUN_TEST(test_malformed_json_is_a_fetch_error);
  return UNITY_END();
}
