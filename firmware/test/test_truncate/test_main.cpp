#include <unity.h>

#include "util/truncate.h"

// One width unit per code point, so the expectations are easy to count.
static int codePoints(const std::string& s) {
  int n = 0;
  for (unsigned char c : s) n += (c & 0xC0) != 0x80;
  return n;
}

void setUp() {}
void tearDown() {}

void test_text_that_fits_is_unchanged() {
  TEST_ASSERT_EQUAL_STRING("Ostbahnhof", truncateToWidth("Ostbahnhof", 10, codePoints).c_str());
}

void test_long_text_ends_with_ellipsis_within_width() {
  TEST_ASSERT_EQUAL_STRING("Haupt…", truncateToWidth("Hauptbahnhof", 6, codePoints).c_str());
}

void test_umlaut_is_never_split() {
  TEST_ASSERT_EQUAL_STRING("Gr…", truncateToWidth("Grünwald", 3, codePoints).c_str());
  TEST_ASSERT_EQUAL_STRING("Grü…", truncateToWidth("Grünwald", 4, codePoints).c_str());
}

void test_empty_when_ellipsis_does_not_fit() {
  TEST_ASSERT_EQUAL_STRING("", truncateToWidth("Hauptbahnhof", 0, codePoints).c_str());
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_text_that_fits_is_unchanged);
  RUN_TEST(test_long_text_ends_with_ellipsis_within_width);
  RUN_TEST(test_umlaut_is_never_split);
  RUN_TEST(test_empty_when_ellipsis_does_not_fit);
  return UNITY_END();
}
