#pragma once

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <optional>

namespace iso8601_detail {

// Days since 1970-01-01 of a proleptic Gregorian date (Howard Hinnant's days_from_civil).
inline int64_t daysFromCivil(int64_t y, unsigned m, unsigned d) {
  y -= m <= 2;
  const int64_t era = (y >= 0 ? y : y - 399) / 400;
  const unsigned yoe = static_cast<unsigned>(y - era * 400);
  const unsigned doy = (153 * (m > 2 ? m - 3 : m + 9) + 2) / 5 + d - 1;
  const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
  return era * 146097 + static_cast<int64_t>(doe) - 719468;
}

}  // namespace iso8601_detail

// Epoch seconds of "YYYY-MM-DDTHH:MM:SS[.fff]" followed by "Z" or "+hh:mm" / "-hh:mm".
// Does not read the process TZ, so host and device agree.
inline std::optional<time_t> parseIso8601(const char* text) {
  int year, month, day, hour, minute, second, consumed = 0;
  if (sscanf(text, "%4d-%2d-%2dT%2d:%2d:%2d%n", &year, &month, &day, &hour, &minute, &second,
             &consumed) != 6 ||
      month < 1 || month > 12 || day < 1 || day > 31 || hour > 23 || minute > 59 || second > 59) {
    return std::nullopt;
  }
  const char* p = text + consumed;
  if (*p == '.') {
    for (p++; *p >= '0' && *p <= '9';) p++;
  }
  int offset = 0;  // seconds east of UTC
  if (strcmp(p, "Z") != 0) {
    int offsetHour, offsetMinute;
    if (strlen(p) != 6 || (p[0] != '+' && p[0] != '-') || p[3] != ':' ||
        sscanf(p + 1, "%2d:%2d", &offsetHour, &offsetMinute) != 2) {
      return std::nullopt;
    }
    offset = (p[0] == '-' ? -1 : 1) * (offsetHour * 3600 + offsetMinute * 60);
  }
  int64_t local =
      iso8601_detail::daysFromCivil(year, month, day) * 86400 + hour * 3600 + minute * 60 + second;
  return static_cast<time_t>(local - offset);
}
