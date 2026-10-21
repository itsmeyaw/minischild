#pragma once

#include <string>

#include "util/percent_encode.h"

// URL of the BVG (HAFAS REST) departures endpoint for a Station, identified by its IBNR.
inline std::string bvgDeparturesUrl(const std::string& stationId) {
  return "https://v6.bvg.transport.rest/stops/" + percentEncode(stationId) +
         "/departures?results=20&duration=60&remarks=false&stopovers=false&linesOfStops=false"
         "&language=de";
}
