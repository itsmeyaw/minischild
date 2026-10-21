#include <unity.h>

#include <string>

#include "provider/bvg/departures_url.h"

void setUp() {}
void tearDown() {}

void test_builds_the_departures_url_for_a_station() {
  TEST_ASSERT_EQUAL_STRING(
      "https://v6.bvg.transport.rest/stops/900100003/departures"
      "?results=20&duration=60&remarks=false&stopovers=false&linesOfStops=false&language=de",
      bvgDeparturesUrl("900100003").c_str());
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_builds_the_departures_url_for_a_station);
  return UNITY_END();
}
