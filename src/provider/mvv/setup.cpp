#include "setup.h"

#include "config.h"
#include "provider/mvv/departure_provider.h"

DepartureProvider& setupDepartureProvider() {
  static MvvDepartureProvider provider(STATION_ID, ROUTE_DIRECTION_IDS);
  return provider;
}
