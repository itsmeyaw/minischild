#pragma once

#include <string>

#include "util/percent_encode.h"

// URL of the EFA XML_DM_REQUEST departures endpoint for a Station, identified by its global ID.
inline std::string efaDeparturesUrl(const std::string& baseUrl, const std::string& stationId,
                                    int limit) {
  return baseUrl +
         "/XML_DM_REQUEST?outputFormat=rapidJSON&type_dm=stop&name_dm=" + percentEncode(stationId) +
         "&mode=direct&useRealtime=1&limit=" + std::to_string(limit);
}
