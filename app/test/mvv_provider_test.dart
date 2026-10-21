import 'dart:io';

import 'package:flutter_test/flutter_test.dart';
import 'package:http/http.dart' as http;
import 'package:http/testing.dart';
import 'package:minischild/mvv_provider.dart';

// Captured from the MVV website and stop finder on 2026-10-21.
void main() {
  test('searchStations keeps only the stops with global IDs', () async {
    final body = File(
      'test/fixtures/mvv/stop_finder_marienpl.json',
    ).readAsBytesSync();
    final client = MockClient(
      (request) async => http.Response.bytes(body, 200),
    );

    final stations = await MvvProvider(client).searchStations('Marienpl');

    expect(stations.map((station) => station.id), [
      'de:09162:2',
      'de:09162:20',
    ]);
    expect(stations.map((station) => station.name), [
      'München, Marienplatz',
      'München, Marienplatz (Theatinerstraße)',
    ]);
  });

  test('searchStations asks the stop finder for the raw query', () async {
    final body = File(
      'test/fixtures/mvv/stop_finder_marienpl.json',
    ).readAsBytesSync();
    late http.Request sent;
    final client = MockClient((request) async {
      sent = request;
      return http.Response.bytes(body, 200);
    });

    await MvvProvider(client).searchStations('Marienpl & Co');

    expect(sent.url.scheme, 'https');
    expect(sent.url.host, 'efa.mvv-muenchen.de');
    expect(sent.url.path, '/ng/XML_STOPFINDER_REQUEST');
    expect(sent.url.queryParameters, {
      'outputFormat': 'rapidJSON',
      'type_sf': 'any',
      'name_sf': 'Marienpl & Co',
    });
  });

  test('routeDirections maps the available lines of a station', () async {
    final body = File(
      'test/fixtures/mvv/available_lines_marienplatz.json',
    ).readAsBytesSync();
    final client = MockClient(
      (request) async => http.Response.bytes(body, 200),
    );

    final directions = await MvvProvider(client).routeDirections('de:09162:2');

    expect(directions.map((direction) => direction.id), [
      'ddb:920S1: :H:j26',
      'ddb:920S1: :R:j26',
      'swm:010U3:G:H:016',
      'swm:010U3:G:R:016',
    ]);
    expect(directions.map((direction) => direction.route), [
      'S1',
      'S1',
      'U3',
      'U3',
    ]);
    expect(directions.map((direction) => direction.headsign), [
      'Leuchtenbergring',
      'Freising',
      'Sendlinger Tor',
      'Moosach',
    ]);
  });

  test('routeDirections asks the departures finder for the station', () async {
    final body = File(
      'test/fixtures/mvv/available_lines_marienplatz.json',
    ).readAsBytesSync();
    late http.Request sent;
    final client = MockClient((request) async {
      sent = request;
      return http.Response.bytes(body, 200);
    });

    await MvvProvider(client).routeDirections('de:09162:2');

    expect(sent.url.scheme, 'https');
    expect(sent.url.host, 'www.mvv-muenchen.de');
    expect(sent.url.path, '/');
    expect(sent.url.queryParameters, {
      'eID': 'departuresFinder',
      'action': 'available_lines',
      'stop_id': 'de:09162:2',
    });
  });

  test('a non-200 answer throws ClientException', () async {
    final client = MockClient((request) async => http.Response('oops', 500));

    expect(
      MvvProvider(client).searchStations('Marienpl'),
      throwsA(isA<http.ClientException>()),
    );
    expect(
      MvvProvider(client).routeDirections('de:09162:2'),
      throwsA(isA<http.ClientException>()),
    );
  });
}
