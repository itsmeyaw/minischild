#pragma once

#include <string>
#include <vector>

#include "provider/departure_provider.h"

class MvvDepartureProvider : public DepartureProvider {
 public:
  MvvDepartureProvider(std::string stationId, std::vector<std::string> routeDirectionIds);

  FetchResult fetch() override;

 private:
  std::string stationId_;
  std::vector<std::string> routeDirectionIds_;
};
