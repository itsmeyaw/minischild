#include <unity.h>

#include <string>
#include <vector>

#include "provider/route_direction.h"

void setUp() {}
void tearDown() {}

void test_route_direction_id_joins_line_and_destination() {
  TEST_ASSERT_EQUAL_STRING("U2|900130002", routeDirectionId("U2", "900130002").c_str());
}

void test_route_direction_id_has_empty_destination_when_unknown() {
  TEST_ASSERT_EQUAL_STRING("U2|", routeDirectionId("U2", "").c_str());
}

void test_empty_selection_selects_every_route_direction() {
  TEST_ASSERT_TRUE(isSelectedRouteDirection({}, "U2|900130002"));
}

void test_selection_selects_a_listed_route_direction() {
  TEST_ASSERT_TRUE(isSelectedRouteDirection({"M6|900100038", "U2|900130002"}, "U2|900130002"));
}

void test_selection_rejects_an_unlisted_route_direction() {
  TEST_ASSERT_FALSE(isSelectedRouteDirection({"M6|900100038"}, "U2|900130002"));
}

void test_selection_matches_exactly_not_by_prefix() {
  TEST_ASSERT_FALSE(isSelectedRouteDirection({"U2|9001"}, "U2|900130002"));
  TEST_ASSERT_FALSE(isSelectedRouteDirection({"U2|900130002"}, "U2|9001"));
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_route_direction_id_joins_line_and_destination);
  RUN_TEST(test_route_direction_id_has_empty_destination_when_unknown);
  RUN_TEST(test_empty_selection_selects_every_route_direction);
  RUN_TEST(test_selection_selects_a_listed_route_direction);
  RUN_TEST(test_selection_rejects_an_unlisted_route_direction);
  RUN_TEST(test_selection_matches_exactly_not_by_prefix);
  return UNITY_END();
}
