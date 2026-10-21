#pragma once

#include <string>
#include <vector>

#include "provider/departure_provider.h"
#include "provider/mvv/route_directions.h"
#include "provider/mvv/stop_name_cache.h"

class MvvDepartureProvider : public DepartureProvider {
 public:
  // An empty routeDirectionIds shows all Routes at the Station.
  MvvDepartureProvider(std::string stationId, std::vector<std::string> routeDirectionIds);
  // Not copyable or movable: the lookups capture this.
  MvvDepartureProvider(const MvvDepartureProvider&) = delete;
  MvvDepartureProvider& operator=(const MvvDepartureProvider&) = delete;

  FetchResult fetch() override;
  void select(const BoardSelection& selection) override;

 private:
  FetchResult fetchBoard();

  std::string stationId_;
  RouteDirections routeDirections_;
  StopNameCache stopName_;
};
