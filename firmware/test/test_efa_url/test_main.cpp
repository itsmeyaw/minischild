#include <unity.h>

#include <string>
#include <vector>

#include "provider/efa/departures_url.h"

void setUp() {}
void tearDown() {}

void test_builds_the_departures_url_for_a_station() {
  TEST_ASSERT_EQUAL_STRING(
      "https://www3.vvs.de/mngvvs/XML_DM_REQUEST?outputFormat=rapidJSON&type_dm=stop"
      "&name_dm=de%3A08111%3A6118&mode=direct&useRealtime=1&limit=20",
      efaDeparturesUrl("https://www3.vvs.de/mngvvs", "de:08111:6118", 20).c_str());
}

void test_efa_limit_is_10_without_route_directions() {
  TEST_ASSERT_EQUAL_INT(10, efaDepartureLimit({}));
}

void test_efa_limit_is_20_with_route_directions() {
  TEST_ASSERT_EQUAL_INT(20, efaDepartureLimit({"S1:1"}));
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_builds_the_departures_url_for_a_station);
  RUN_TEST(test_efa_limit_is_10_without_route_directions);
  RUN_TEST(test_efa_limit_is_20_with_route_directions);
  return UNITY_END();
}
