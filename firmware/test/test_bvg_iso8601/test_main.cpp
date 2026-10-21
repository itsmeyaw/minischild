#include <unity.h>

#include <ctime>
#include <optional>

#include "provider/bvg/iso8601.h"

void setUp() {}
void tearDown() {}

void test_winter_offset_is_cet() {
  TEST_ASSERT_EQUAL(1768474800, parseIso8601("2026-01-15T12:00:00+01:00").value());
}

void test_summer_offset_is_cest() {
  TEST_ASSERT_EQUAL(1784109600, parseIso8601("2026-07-15T12:00:00+02:00").value());
}

void test_z_suffix_is_utc() {
  TEST_ASSERT_EQUAL(1784109600, parseIso8601("2026-07-15T10:00:00Z").value());
}

void test_negative_offset_is_west_of_utc() {
  TEST_ASSERT_EQUAL(1792599240, parseIso8601("2026-10-21T11:14:00-05:00").value());
}

void test_fractional_seconds_are_ignored() {
  TEST_ASSERT_EQUAL(1784109600, parseIso8601("2026-07-15T12:00:00.000+02:00").value());
}

// 2026-03-29 clocks go forward at 02:00 CET: these are one second apart in UTC.
void test_offsets_across_the_dst_change_are_exact() {
  TEST_ASSERT_EQUAL(1774745999, parseIso8601("2026-03-29T01:59:59+01:00").value());
  TEST_ASSERT_EQUAL(1774746000, parseIso8601("2026-03-29T03:00:00+02:00").value());
}

void test_invalid_input_is_nullopt() {
  TEST_ASSERT_FALSE(parseIso8601("").has_value());
  TEST_ASSERT_FALSE(parseIso8601("garbage").has_value());
  TEST_ASSERT_FALSE(parseIso8601("2026-10-21").has_value());
  TEST_ASSERT_FALSE(parseIso8601("2026-10-21T18:14:00").has_value());
  TEST_ASSERT_FALSE(parseIso8601("2026-13-21T18:14:00+02:00").has_value());
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_winter_offset_is_cet);
  RUN_TEST(test_summer_offset_is_cest);
  RUN_TEST(test_z_suffix_is_utc);
  RUN_TEST(test_negative_offset_is_west_of_utc);
  RUN_TEST(test_fractional_seconds_are_ignored);
  RUN_TEST(test_offsets_across_the_dst_change_are_exact);
  RUN_TEST(test_invalid_input_is_nullopt);
  return UNITY_END();
}
