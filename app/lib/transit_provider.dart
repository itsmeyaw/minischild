class Station {
  final String id;
  final String name;

  const Station({required this.id, required this.name});
}

class RouteDirection {
  final String id;
  final String route;
  final String headsign;

  const RouteDirection({
    required this.id,
    required this.route,
    required this.headsign,
  });
}

abstract interface class TransitProvider {
  Future<List<Station>> searchStations(String query);

  Future<List<RouteDirection>> routeDirections(String stationId);
}

typedef ProviderLookup = TransitProvider? Function(String providerName);
