#include <unity.h>

#include <string>

#include "util/percent_encode.h"

void setUp() {}
void tearDown() {}

void test_unreserved_characters_are_kept() {
  TEST_ASSERT_EQUAL_STRING("AZaz09-._~", percentEncode("AZaz09-._~").c_str());
}

void test_space_becomes_percent_20() {
  TEST_ASSERT_EQUAL_STRING("a%20b", percentEncode("a b").c_str());
}

void test_colon_becomes_percent_3A() {
  TEST_ASSERT_EQUAL_STRING("de%3A09162", percentEncode("de:09162").c_str());
}

void test_utf8_bytes_become_uppercase_hex() {
  // "ü" is 0xC3 0xBC in UTF-8.
  TEST_ASSERT_EQUAL_STRING("Gr%C3%BC", percentEncode("Grü").c_str());
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_unreserved_characters_are_kept);
  RUN_TEST(test_space_becomes_percent_20);
  RUN_TEST(test_colon_becomes_percent_3A);
  RUN_TEST(test_utf8_bytes_become_uppercase_hex);
  return UNITY_END();
}
