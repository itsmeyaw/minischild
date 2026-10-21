#pragma once

#include <string>
#include <vector>

#include "util/percent_encode.h"

// The server has no usable line filter, so Route Directions are filtered on the Device; 20 leaves
// enough after filtering, and 10 keeps the unfiltered payload at ~24-60 KB (20: ~40-125 KB).
inline constexpr int kEfaLimitUnfiltered = 10;
inline constexpr int kEfaLimitFiltered = 20;

inline int efaDepartureLimit(const std::vector<std::string>& routeDirectionIds) {
  return routeDirectionIds.empty() ? kEfaLimitUnfiltered : kEfaLimitFiltered;
}

// URL of the EFA XML_DM_REQUEST departures endpoint for a Station, identified by its global ID.
inline std::string efaDeparturesUrl(const std::string& baseUrl, const std::string& stationId,
                                    int limit) {
  return baseUrl +
         "/XML_DM_REQUEST?outputFormat=rapidJSON&type_dm=stop&name_dm=" + percentEncode(stationId) +
         "&mode=direct&useRealtime=1&limit=" + std::to_string(limit);
}
