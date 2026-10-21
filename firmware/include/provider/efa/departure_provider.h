#pragma once

#include <ctime>
#include <string>
#include <utility>
#include <vector>

#include "provider/departure_provider.h"
#include "provider/efa/departure_parser.h"
#include "provider/efa/departures_url.h"
#include "util/https_json.h"

struct EfaInstance {
  const char* baseUrl;
  const char* rootCa;
};

class EfaDepartureProvider : public DepartureProvider {
 public:
  // An empty routeDirectionIds shows all Routes at the Station.
  EfaDepartureProvider(EfaInstance instance, std::string stationId,
                       std::vector<std::string> routeDirectionIds)
      : instance_(instance),
        stationId_(std::move(stationId)),
        routeDirectionIds_(std::move(routeDirectionIds)) {}

  FetchResult fetch() override {
    const time_t now = time(nullptr);
    return getJson<FetchResult>(
        instance_.rootCa,
        efaDeparturesUrl(instance_.baseUrl, stationId_, efaDepartureLimit(routeDirectionIds_)),
        [now, this](WaitingReader& body) {
          return parseEfaDepartures(body, now, routeDirectionIds_);
        });
  }

 private:
  EfaInstance instance_;
  std::string stationId_;
  std::vector<std::string> routeDirectionIds_;
};
