#include "setup.h"

#include "provider/efa/departure_provider.h"
#include "provider/efa/digicert_global_root_g2.h"

namespace {
constexpr EfaInstance kInstance{"https://efa.vrr.de/standard", kDigiCertGlobalRootG2};
}  // namespace

DepartureProvider& setupDepartureProvider(const BoardSelection& selection) {
  static EfaDepartureProvider provider(kInstance, selection.stationId, selection.routeDirectionIds);
  return provider;
}
