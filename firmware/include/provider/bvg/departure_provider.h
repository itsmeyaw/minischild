#pragma once

#include <string>
#include <vector>

#include "provider/departure_provider.h"

class BvgDepartureProvider : public DepartureProvider {
 public:
  // An empty routeDirectionIds shows all Routes at the Station.
  BvgDepartureProvider(std::string stationId, std::vector<std::string> routeDirectionIds);

  FetchResult fetch() override;

 private:
  std::string stationId_;
  std::vector<std::string> routeDirectionIds_;
};
