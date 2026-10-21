#include <unity.h>

#include <string>
#include <vector>

#include "provider/mvv/route_directions.h"

void setUp() {}

void tearDown() {}

void test_configured_route_directions_are_returned_without_lookup() {
  int lookups = 0;
  RouteDirections routeDirections({"ID1", "ID2"}, [&] {
    lookups++;
    return RouteDirectionsResult{std::vector<std::string>{"LOOKED_UP"}};
  });

  RouteDirectionsResult result = routeDirections.get();

  TEST_ASSERT_EQUAL(0, lookups);
  TEST_ASSERT_EQUAL(2, std::get<std::vector<std::string>>(result).size());
  TEST_ASSERT_EQUAL_STRING("ID1", std::get<std::vector<std::string>>(result)[0].c_str());
  TEST_ASSERT_EQUAL_STRING("ID2", std::get<std::vector<std::string>>(result)[1].c_str());
}

void test_empty_config_looks_up_once_and_reuses_the_result() {
  int lookups = 0;
  RouteDirections routeDirections({}, [&] {
    lookups++;
    return RouteDirectionsResult{std::vector<std::string>{"ID1", "ID2"}};
  });

  for (int i = 0; i < 3; i++) {
    RouteDirectionsResult result = routeDirections.get();
    TEST_ASSERT_EQUAL(2, std::get<std::vector<std::string>>(result).size());
    TEST_ASSERT_EQUAL_STRING("ID1", std::get<std::vector<std::string>>(result)[0].c_str());
    TEST_ASSERT_EQUAL_STRING("ID2", std::get<std::vector<std::string>>(result)[1].c_str());
  }

  TEST_ASSERT_EQUAL(1, lookups);
}

void test_failed_lookup_is_returned_and_retried_on_the_next_get() {
  int lookups = 0;
  RouteDirections routeDirections({}, [&] {
    lookups++;
    if (lookups == 1) return RouteDirectionsResult{FetchError{"MVV: boom"}};
    return RouteDirectionsResult{std::vector<std::string>{"ID1"}};
  });

  RouteDirectionsResult first = routeDirections.get();
  TEST_ASSERT_TRUE(std::holds_alternative<FetchError>(first));
  TEST_ASSERT_EQUAL_STRING("MVV: boom", std::get<FetchError>(first).message.c_str());

  RouteDirectionsResult second = routeDirections.get();
  TEST_ASSERT_EQUAL_STRING("ID1", std::get<std::vector<std::string>>(second)[0].c_str());

  routeDirections.get();
  TEST_ASSERT_EQUAL(2, lookups);
}

void test_empty_lookup_result_is_cached_as_an_unknown_station() {
  int lookups = 0;
  RouteDirections routeDirections({}, [&] {
    lookups++;
    return RouteDirectionsResult{std::vector<std::string>{}};
  });

  RouteDirectionsResult first = routeDirections.get();
  RouteDirectionsResult second = routeDirections.get();

  TEST_ASSERT_EQUAL(1, lookups);
  TEST_ASSERT_TRUE(std::get<std::vector<std::string>>(first).empty());
  TEST_ASSERT_TRUE(std::get<std::vector<std::string>>(second).empty());
}

void test_select_replaces_the_configured_route_directions() {
  RouteDirections routeDirections(
      {"ID1"}, [] { return RouteDirectionsResult{std::vector<std::string>{"LOOKED_UP"}}; });

  routeDirections.select({"ID2", "ID3"});

  RouteDirectionsResult result = routeDirections.get();
  TEST_ASSERT_EQUAL(2, std::get<std::vector<std::string>>(result).size());
  TEST_ASSERT_EQUAL_STRING("ID2", std::get<std::vector<std::string>>(result)[0].c_str());
  TEST_ASSERT_EQUAL_STRING("ID3", std::get<std::vector<std::string>>(result)[1].c_str());
}

void test_select_empty_looks_up_again_instead_of_returning_the_old_cache() {
  int lookups = 0;
  RouteDirections routeDirections({}, [&] {
    lookups++;
    return RouteDirectionsResult{std::vector<std::string>{lookups == 1 ? "OLD" : "NEW"}};
  });
  TEST_ASSERT_EQUAL_STRING("OLD",
                           std::get<std::vector<std::string>>(routeDirections.get())[0].c_str());

  routeDirections.select({});

  RouteDirectionsResult result = routeDirections.get();
  TEST_ASSERT_EQUAL(2, lookups);
  TEST_ASSERT_EQUAL_STRING("NEW", std::get<std::vector<std::string>>(result)[0].c_str());
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_configured_route_directions_are_returned_without_lookup);
  RUN_TEST(test_empty_config_looks_up_once_and_reuses_the_result);
  RUN_TEST(test_failed_lookup_is_returned_and_retried_on_the_next_get);
  RUN_TEST(test_empty_lookup_result_is_cached_as_an_unknown_station);
  RUN_TEST(test_select_replaces_the_configured_route_directions);
  RUN_TEST(test_select_empty_looks_up_again_instead_of_returning_the_old_cache);
  return UNITY_END();
}
