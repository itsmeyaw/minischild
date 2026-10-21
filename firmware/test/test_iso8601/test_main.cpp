#include <unity.h>

#include <ctime>
#include <optional>

#include "util/iso8601.h"

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

void test_days_beyond_the_month_length_are_nullopt() {
  TEST_ASSERT_FALSE(parseIso8601("2026-02-29T00:00:00Z").has_value());
  TEST_ASSERT_FALSE(parseIso8601("2026-04-31T00:00:00Z").has_value());
  TEST_ASSERT_FALSE(parseIso8601("2100-02-29T00:00:00Z").has_value());
}

void test_leap_days_are_valid() {
  TEST_ASSERT_EQUAL(1835395200, parseIso8601("2028-02-29T00:00:00Z").value());
  TEST_ASSERT_EQUAL(951782400, parseIso8601("2000-02-29T00:00:00Z").value());
}

void test_offset_minutes_above_59_are_nullopt() {
  TEST_ASSERT_FALSE(parseIso8601("2026-10-21T18:14:00+02:60").has_value());
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
  RUN_TEST(test_days_beyond_the_month_length_are_nullopt);
  RUN_TEST(test_leap_days_are_valid);
  RUN_TEST(test_offset_minutes_above_59_are_nullopt);
  return UNITY_END();
}
