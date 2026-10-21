#include <unity.h>

#include <string>

#include "provider/mvv/stop_name_cache.h"

void setUp() {}

void tearDown() {}

void test_looks_up_the_name_once_and_reuses_it() {
  int lookups = 0;
  StopNameCache stopName([&] {
    lookups++;
    return StopNameResult{std::string("Marienplatz")};
  });

  for (int i = 0; i < 3; i++) {
    TEST_ASSERT_EQUAL_STRING("Marienplatz", stopName.get().c_str());
  }

  TEST_ASSERT_EQUAL(1, lookups);
}

void test_failed_lookup_gives_an_empty_name_and_is_retried_on_the_next_get() {
  int lookups = 0;
  StopNameCache stopName([&] {
    lookups++;
    if (lookups == 1) return StopNameResult{FetchError{"MVV: boom"}};
    return StopNameResult{std::string("Marienplatz")};
  });

  TEST_ASSERT_EQUAL_STRING("", stopName.get().c_str());
  TEST_ASSERT_EQUAL_STRING("Marienplatz", stopName.get().c_str());
  TEST_ASSERT_EQUAL_STRING("Marienplatz", stopName.get().c_str());
  TEST_ASSERT_EQUAL(2, lookups);
}

void test_reset_looks_the_name_up_again() {
  int lookups = 0;
  StopNameCache stopName([&] {
    lookups++;
    return StopNameResult{std::string(lookups == 1 ? "Marienplatz" : "Sendlinger Tor")};
  });
  TEST_ASSERT_EQUAL_STRING("Marienplatz", stopName.get().c_str());

  stopName.reset();

  TEST_ASSERT_EQUAL_STRING("Sendlinger Tor", stopName.get().c_str());
  TEST_ASSERT_EQUAL(2, lookups);
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_looks_up_the_name_once_and_reuses_it);
  RUN_TEST(test_failed_lookup_gives_an_empty_name_and_is_retried_on_the_next_get);
  RUN_TEST(test_reset_looks_the_name_up_again);
  return UNITY_END();
}
