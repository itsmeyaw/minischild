#pragma once

#include <string>
#include <vector>

#include "provider/departure_provider.h"
#include "provider/mvv/route_directions.h"

class MvvDepartureProvider : public DepartureProvider {
 public:
  // An empty routeDirectionIds shows all Routes at the Station.
  MvvDepartureProvider(std::string stationId, std::vector<std::string> routeDirectionIds);

  FetchResult fetch() override;

 private:
  std::string stationId_;
  RouteDirections routeDirections_;
};
