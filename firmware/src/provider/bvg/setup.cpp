#include "setup.h"

#include "provider/bvg/departure_provider.h"

DepartureProvider& setupDepartureProvider(const BoardSelection& selection) {
  static BvgDepartureProvider provider(selection.stationId, selection.routeDirectionIds);
  return provider;
}
