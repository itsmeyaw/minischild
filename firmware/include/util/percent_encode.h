#pragma once

#include <string>

// Percent-encodes every byte outside the RFC 3986 unreserved set, so UTF-8 becomes %XX per byte.
inline std::string percentEncode(const std::string& s) {
  static const char kHex[] = "0123456789ABCDEF";
  std::string out;
  for (unsigned char c : s) {
    bool unreserved = (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') ||
                      c == '-' || c == '.' || c == '_' || c == '~';
    if (unreserved) {
      out += static_cast<char>(c);
    } else {
      out += '%';
      out += kHex[c >> 4];
      out += kHex[c & 0xF];
    }
  }
  return out;
}
