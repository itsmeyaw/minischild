#include <unity.h>

#include <string>

#include "provider/efa/departures_url.h"

void setUp() {}
void tearDown() {}

void test_builds_the_departures_url_for_a_station() {
  TEST_ASSERT_EQUAL_STRING(
      "https://www3.vvs.de/mngvvs/XML_DM_REQUEST?outputFormat=rapidJSON&type_dm=stop"
      "&name_dm=de%3A08111%3A6118&mode=direct&useRealtime=1&limit=20",
      efaDeparturesUrl("https://www3.vvs.de/mngvvs", "de:08111:6118", 20).c_str());
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_builds_the_departures_url_for_a_station);
  return UNITY_END();
}
