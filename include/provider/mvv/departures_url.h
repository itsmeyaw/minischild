#pragma once

#include <cstdint>
#include <ctime>
#include <string>
#include <vector>

namespace mvv_url_detail {

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

inline std::string base64(const std::string& in) {
  static const char kAlphabet[] =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  std::string out;
  for (size_t i = 0; i < in.size(); i += 3) {
    size_t left = in.size() - i;
    uint32_t chunk = static_cast<uint32_t>(static_cast<unsigned char>(in[i])) << 16;
    if (left > 1) chunk |= static_cast<uint32_t>(static_cast<unsigned char>(in[i + 1])) << 8;
    if (left > 2) chunk |= static_cast<unsigned char>(in[i + 2]);
    for (size_t k = 0; k < 4; k++) {
      out += k <= left ? kAlphabet[(chunk >> (18 - 6 * k)) & 0x3F] : '=';
    }
  }
  return out;
}

}  // namespace mvv_url_detail

// URL of the MVV website's departures endpoint for a Station and the Route Directions to show.
inline std::string mvvDeparturesUrl(const std::string& stationId,
                                    const std::vector<std::string>& routeDirectionIds,
                                    time_t requestedTimestamp) {
  std::string lines;
  for (const std::string& id : routeDirectionIds) {
    lines += "&line=" + mvv_url_detail::percentEncode(id);
  }
  return "https://www.mvv-muenchen.de/?eID=departuresFinder&action=get_departures&stop_id=" +
         mvv_url_detail::percentEncode(stationId) +
         "&requested_timestamp=" + std::to_string(requestedTimestamp) +
         "&lines=" + mvv_url_detail::percentEncode(mvv_url_detail::base64(lines));
}

// URL of the MVV website's endpoint listing the Route Directions of a Station.
inline std::string mvvAvailableLinesUrl(const std::string& stationId) {
  return "https://www.mvv-muenchen.de/?eID=departuresFinder&action=available_lines&stop_id=" +
         mvv_url_detail::percentEncode(stationId);
}
