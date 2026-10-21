import 'package:flutter/material.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:minischild/app.dart';
import 'package:minischild/device_link.dart';
import 'package:minischild/station_search_screen.dart';
import 'package:minischild/transit_provider.dart';

import 'fakes.dart';

const _device = FoundDevice(id: 'a', name: 'Minischild-1F2A');
const _marienplatz = Station(id: 'de:09162:2', name: 'München, Marienplatz');
const _theatiner = Station(
  id: 'de:09162:20',
  name: 'München, Marienplatz (Theatinerstraße)',
);

void main() {
  final info = exampleInfo();
  final settings = exampleSettings('settings-read.json');
  FakeTransitProvider mvvProvider() => FakeTransitProvider(
    stationResults: const [_marienplatz, _theatiner],
    routeResults: const [
      RouteDirection(
        id: 'ddb:920S1: :H:j26',
        route: 'S1',
        headsign: 'Freising',
      ),
    ],
  );

  Future<FakeDeviceLink> openDevice(
    WidgetTester tester, {
    FakeTransitProvider? mvv,
    ProviderLookup? providerFor,
  }) async {
    final provider = mvv ?? mvvProvider();
    final link = FakeDeviceLink(info: info, settings: settings);
    await tester.pumpWidget(
      CompanionApp(
        link: link,
        providerFor: providerFor ?? (name) => name == 'mvv' ? provider : null,
      ),
    );
    link.emitDevices(const [_device]);
    await tester.pump();
    await tester.tap(find.text(_device.name));
    await tester.pump(const Duration(seconds: 1));
    await tester.pump();
    return link;
  }

  Future<void> openStationSearch(WidgetTester tester) async {
    await tester.tap(find.widgetWithText(ListTile, 'Station'));
    await tester.pump(const Duration(seconds: 1));
    await tester.pump();
  }

  Future<void> search(WidgetTester tester, String text) async {
    await tester.enterText(find.byType(TextField), text);
    await tester.pump(const Duration(milliseconds: 400));
    await tester.pump();
  }

  testWidgets(
    'choosing a Station by search saves it and the Device keeps the connection',
    (tester) async {
      final link = await openDevice(tester);
      await openStationSearch(tester);

      await search(tester, 'Marienpl');
      expect(
        find.text('München, Marienplatz (Theatinerstraße)'),
        findsOneWidget,
      );
      expect(_saveButton(tester).onPressed, isNull);

      await tester.tap(find.text('München, Marienplatz (Theatinerstraße)'));
      await tester.pump();
      expect(_saveButton(tester).onPressed, isNotNull);

      await tester.tap(find.text('Save'));
      await tester.pump(const Duration(seconds: 1));
      await tester.pump(const Duration(seconds: 1));

      expect(link.writes, hasLength(1));
      final write = link.writes.single;
      expect(write.settings.stationId, 'de:09162:20');
      expect(write.settings.routeDirectionIds, isEmpty);
      expect(write.settings.platforms, isEmpty);
      expect(write.settings.wifiSsid, settings.wifiSsid);
      expect(write.wifiPassword, '');
      expect(find.text('The Device is restarting'), findsNothing);
      expect(link.connectCount, 1);
      expect(link.disconnectCount, 0);
      expect(
        find.text('München, Marienplatz (Theatinerstraße)'),
        findsOneWidget,
      );
    },
  );

  testWidgets(
    'choosing the current Station again keeps its Routes and Platforms',
    (tester) async {
      final link = await openDevice(tester);
      await openStationSearch(tester);

      await search(tester, 'Marienpl');
      await tester.tap(
        find.descendant(
          of: find.byType(StationSearchScreen),
          matching: find.text('München, Marienplatz'),
        ),
      );
      await tester.pump();
      await tester.tap(find.text('Save'));
      await tester.pump(const Duration(seconds: 1));
      await tester.pump(const Duration(seconds: 1));

      final write = link.writes.single;
      expect(write.settings.stationId, 'de:09162:2');
      expect(write.settings.routeDirectionIds, settings.routeDirectionIds);
      expect(write.settings.platforms, settings.platforms);
      await tester.pump(const Duration(seconds: 3));
      await tester.pump();
    },
  );

  testWidgets('the query is the typed text, sent once after typing pauses', (
    tester,
  ) async {
    final provider = mvvProvider();
    await openDevice(tester, mvv: provider);
    await openStationSearch(tester);
    provider.queries.clear();

    await tester.enterText(find.byType(TextField), 'M');
    await tester.pump(const Duration(milliseconds: 200));
    await tester.enterText(find.byType(TextField), 'Marien');
    await tester.pump(const Duration(milliseconds: 399));
    expect(provider.queries, isEmpty);
    await tester.pump(const Duration(milliseconds: 1));
    await tester.pump();

    expect(provider.queries, ['Marien']);
  });

  testWidgets('submitting searches at once and trims the query', (
    tester,
  ) async {
    final provider = mvvProvider();
    await openDevice(tester, mvv: provider);
    await openStationSearch(tester);
    provider.queries.clear();

    await tester.enterText(find.byType(TextField), '  Garching ');
    await tester.testTextInput.receiveAction(TextInputAction.search);
    await tester.pump();

    expect(provider.queries, ['Garching']);
  });

  testWidgets('when the provider cannot be reached, Try again searches again', (
    tester,
  ) async {
    final provider = mvvProvider();
    await openDevice(tester, mvv: provider);
    await openStationSearch(tester);
    provider.error = Exception('offline');
    provider.queries.clear();

    await search(tester, 'Marienpl');

    expect(find.text('Couldn’t reach the provider'), findsOneWidget);
    expect(
      find.text('Check your phone’s internet connection, then try again.'),
      findsOneWidget,
    );

    provider.error = null;
    await tester.tap(find.widgetWithText(FilledButton, 'Try again'));
    await tester.pump();
    await tester.pump();

    expect(provider.queries, ['Marienpl', 'Marienpl']);
    expect(find.text('Couldn’t reach the provider'), findsNothing);
    expect(find.text('München, Marienplatz (Theatinerstraße)'), findsOneWidget);
  });

  testWidgets('a query without matches says so', (tester) async {
    await openDevice(
      tester,
      mvv: FakeTransitProvider(stationResults: const []),
    );
    await openStationSearch(tester);

    await search(tester, 'Nowhere');

    expect(find.text('No Stations found'), findsOneWidget);
    expect(
      find.text(
        'No Station matches “Nowhere”. Try a shorter or different name.',
      ),
      findsOneWidget,
    );
  });

  testWidgets('a failed save says so and keeps the Station screen open', (
    tester,
  ) async {
    final link = await openDevice(tester);
    link.writeError = const DeviceLinkException(
      DeviceLinkError.connectionFailed,
    );
    await openStationSearch(tester);
    await search(tester, 'Marienpl');
    await tester.tap(find.text('München, Marienplatz (Theatinerstraße)'));
    await tester.pump();

    await tester.tap(find.text('Save'));
    await tester.pump();
    await tester.pump();

    expect(
      find.descendant(
        of: find.byType(StationSearchScreen),
        matching: find.text(
          'Couldn’t save. Make sure the Device is on and nearby, then try again.',
        ),
      ),
      findsOneWidget,
    );
    expect(find.widgetWithText(AppBar, 'Station'), findsOneWidget);
    expect(find.text('The Device is restarting'), findsNothing);
    expect(link.writes, isEmpty);
    expect(_saveButton(tester).onPressed, isNotNull);
  });

  testWidgets('a Device with an unsupported provider cannot choose a Station', (
    tester,
  ) async {
    await openDevice(tester, providerFor: (_) => null);

    expect(find.widgetWithText(ListTile, 'Station'), findsNothing);
  });
}

TextButton _saveButton(WidgetTester tester) =>
    tester.widget<TextButton>(find.widgetWithText(TextButton, 'Save'));
