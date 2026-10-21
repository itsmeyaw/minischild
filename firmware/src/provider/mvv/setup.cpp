#include "setup.h"

#include "provider/mvv/departure_provider.h"

DepartureProvider& setupDepartureProvider(const BoardSelection& selection) {
  static MvvDepartureProvider provider(selection.stationId, selection.routeDirectionIds);
  return provider;
}
