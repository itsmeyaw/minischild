import 'dart:convert';

import 'package:http/http.dart' as http;

import 'transit_provider.dart';

class MvvProvider implements TransitProvider {
  MvvProvider(this._client);

  final http.Client _client;

  @override
  Future<List<Station>> searchStations(String query) async {
    final json = await _getObject(
      Uri.https('efa.mvv-muenchen.de', '/ng/XML_STOPFINDER_REQUEST', {
        'outputFormat': 'rapidJSON',
        'type_sf': 'any',
        'name_sf': query,
      }),
    );
    return switch (json) {
      {'locations': List<Object?> locations} => [
        for (final location in locations)
          if (location case {
            'type': 'stop',
            'isGlobalId': true,
            'id': String id,
            'name': String name,
          })
            Station(id: id, name: name),
      ],
      _ => throw const FormatException('stop finder has no locations'),
    };
  }

  @override
  Future<List<RouteDirection>> routeDirections(String stationId) async {
    final json = await _getObject(
      Uri.https('www.mvv-muenchen.de', '/', {
        'eID': 'departuresFinder',
        'action': 'available_lines',
        'stop_id': stationId,
      }),
    );
    return switch (json) {
      {'lines': List<Object?> lines} => [
        for (final routeDirection in lines)
          if (routeDirection case {
            'stateless': String id,
            'number': String route,
            'direction': String headsign,
          })
            RouteDirection(id: id, route: route, headsign: headsign),
      ],
      _ => throw const FormatException('available lines has no lines'),
    };
  }

  Future<Map<String, Object?>> _getObject(Uri uri) async {
    final response = await _client.get(uri);
    if (response.statusCode != 200) {
      throw http.ClientException('MVV answered ${response.statusCode}', uri);
    }
    final json = jsonDecode(utf8.decode(response.bodyBytes));
    if (json is! Map<String, Object?>) {
      throw const FormatException('MVV response is not a JSON object');
    }
    return json;
  }
}
