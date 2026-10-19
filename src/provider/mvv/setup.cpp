#include "setup.h"

#include <iterator>

#include "config.h"
#include "provider/mvv/departure_provider.h"

DepartureProvider& setupDepartureProvider() {
  static MvvDepartureProvider provider(
      STATION_ID, {std::begin(ROUTE_DIRECTION_IDS), std::end(ROUTE_DIRECTION_IDS)});
  return provider;
}
