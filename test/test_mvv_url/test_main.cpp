#include <unity.h>

#include <ctime>
#include <string>
#include <vector>

#include "provider/mvv/departures_url.h"

// 2026-10-19 12:00:00 UTC
static const time_t NOON = 1792411200;

void setUp() {}
void tearDown() {}

void test_builds_the_url_for_two_route_directions() {
  std::string url =
      mvvDeparturesUrl("de:09162:2", {"ddb:920S1: :H:j26", "swm:010U3:G:H:016"}, NOON);
  TEST_ASSERT_EQUAL_STRING(
      "https://www.mvv-muenchen.de/"
      "?eID=departuresFinder&action=get_departures&stop_id=de%3A09162%3A2"
      "&requested_timestamp=1792411200&lines="
      "JmxpbmU9ZGRiJTNBOTIwUzElM0ElMjAlM0FIJTNBajI2JmxpbmU9c3dtJTNBMDEwVTMlM0FHJTNBSCUzQTAxNg%3D%"
      "3D",
      url.c_str());
}

void test_builds_the_url_for_a_single_route_direction() {
  std::string url = mvvDeparturesUrl("de:09162:2", {"ddb:920S1: :H:j26"}, NOON);
  TEST_ASSERT_EQUAL_STRING(
      "https://www.mvv-muenchen.de/"
      "?eID=departuresFinder&action=get_departures&stop_id=de%3A09162%3A2"
      "&requested_timestamp=1792411200&lines=JmxpbmU9ZGRiJTNBOTIwUzElM0ElMjAlM0FIJTNBajI2",
      url.c_str());
}

void test_pads_base64_with_a_single_equals_sign() {
  std::string url = mvvDeparturesUrl("de:09162:2", {"ab"}, NOON);
  TEST_ASSERT_EQUAL_STRING(
      "https://www.mvv-muenchen.de/"
      "?eID=departuresFinder&action=get_departures&stop_id=de%3A09162%3A2"
      "&requested_timestamp=1792411200&lines=JmxpbmU9YWI%3D",
      url.c_str());
}

// A raw "+" in the query would reach MVV as a space.
void test_percent_encodes_base64_plus() {
  std::string url = mvvDeparturesUrl("de:09162:2", {"aa~"}, NOON);
  TEST_ASSERT_EQUAL_STRING(
      "https://www.mvv-muenchen.de/"
      "?eID=departuresFinder&action=get_departures&stop_id=de%3A09162%3A2"
      "&requested_timestamp=1792411200&lines=JmxpbmU9YWF%2B",
      url.c_str());
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_builds_the_url_for_two_route_directions);
  RUN_TEST(test_builds_the_url_for_a_single_route_direction);
  RUN_TEST(test_pads_base64_with_a_single_equals_sign);
  RUN_TEST(test_percent_encodes_base64_plus);
  return UNITY_END();
}
