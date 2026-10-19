#pragma once

#include <cstddef>
#include <functional>
#include <string>

// Shortens UTF-8 text to fit maxWidth, ending in "…" when cut. Never splits a code point.
inline std::string truncateToWidth(const std::string& text, int maxWidth,
                                   const std::function<int(const std::string&)>& width) {
  if (width(text) <= maxWidth) return text;
  static const std::string ellipsis = "…";
  if (width(ellipsis) > maxWidth) return "";
  std::string cut = text;
  while (!cut.empty() && width(cut + ellipsis) > maxWidth) {
    size_t end = cut.size();
    do --end;
    while (end > 0 && (static_cast<unsigned char>(cut[end]) & 0xC0) == 0x80);
    cut.resize(end);
  }
  return cut + ellipsis;
}
