import 'dart:async';

import 'package:flutter/material.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:minischild/app.dart';
import 'package:minischild/ble_protocol.dart';
import 'package:minischild/device_link.dart';
import 'package:minischild/device_screen.dart';
import 'package:minischild/transit_provider.dart';

import 'fakes.dart';

const _device = FoundDevice(id: 'a', name: 'Minischild-1F2A');
const _restartDelay = Duration(seconds: 3);
const _connectFailure = DeviceLinkException(DeviceLinkError.connectionFailed);

void main() {
  final info = exampleInfo();
  final settings = exampleSettings('settings-read.json');
  FakeTransitProvider mvvProvider() => FakeTransitProvider(
    stationResults: const [
      Station(id: 'de:09162:6', name: 'Garching'),
      Station(id: 'de:09162:2', name: 'München, Marienplatz'),
    ],
    routeResults: const [
      RouteDirection(
        id: 'ddb:920S1: :H:j26',
        route: 'S1',
        headsign: 'Freising',
      ),
      RouteDirection(
        id: 'ddb:920S1: :R:j26',
        route: 'S1',
        headsign: 'Leuchtenbergring',
      ),
    ],
  );

  Future<FakeDeviceLink> openDevice(
    WidgetTester tester, {
    DeviceInfo? deviceInfo,
    Settings? deviceSettings,
    FakeTransitProvider? mvv,
    ProviderLookup? providerFor,
  }) async {
    final provider = mvv ?? mvvProvider();
    final link = FakeDeviceLink(
      info: deviceInfo ?? info,
      settings: deviceSettings ?? settings,
    );
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

  testWidgets('tapping a Device shows its Board Selection and Device details', (
    tester,
  ) async {
    final link = await openDevice(tester);

    expect(link.connectCount, 1);
    expect(find.text('München, Marienplatz'), findsOneWidget);
    expect(find.text('S1 towards Leuchtenbergring'), findsOneWidget);
    expect(find.text('S1 towards Freising'), findsOneWidget);
    expect(find.text('Platform 1'), findsOneWidget);
    expect(find.text('Platform 2'), findsOneWidget);
    await tester.scrollUntilVisible(
      find.text('v0.1.0'),
      200,
      scrollable: find.byType(Scrollable).last,
    );
    expect(find.text('home'), findsOneWidget);
    expect(find.text('mvv'), findsOneWidget);
    expect(find.text('v0.1.0'), findsOneWidget);
  });

  testWidgets('empty Route Direction and Platform lists show all of them', (
    tester,
  ) async {
    await openDevice(
      tester,
      deviceSettings: exampleSettings('settings-write-keep-password.json'),
    );

    expect(find.text('Garching'), findsOneWidget);
    expect(find.text('All Route Directions'), findsOneWidget);
    expect(find.text('All Platforms'), findsOneWidget);
  });

  testWidgets('a Device with an unknown provider shows no Board Selection', (
    tester,
  ) async {
    final mvv = mvvProvider();
    await openDevice(
      tester,
      mvv: mvv,
      deviceInfo: const DeviceInfo(provider: 'bvg', firmware: 'v0.1.0'),
    );

    expect(find.text('Unsupported provider'), findsOneWidget);
    expect(
      find.text(
        'This app can’t show the Board Selection for the “bvg” provider. You can still change the WiFi network.',
      ),
      findsOneWidget,
    );
    expect(find.text('home'), findsOneWidget);
    expect(mvv.calls, 0);
  });

  testWidgets('when the provider cannot be reached, raw IDs are shown', (
    tester,
  ) async {
    await openDevice(
      tester,
      providerFor: (_) => FakeTransitProvider(error: Exception('offline')),
    );

    expect(
      find.text('Couldn’t reach the provider, so IDs are shown.'),
      findsOneWidget,
    );
    expect(find.text('de:09162:2'), findsOneWidget);
    expect(find.text('ddb:920S1: :H:j26'), findsOneWidget);
    expect(find.text('Platform 1'), findsOneWidget);
  });

  testWidgets('losing the connection says so, and Reconnect connects again', (
    tester,
  ) async {
    final link = await openDevice(tester);

    link.loseConnection();
    await tester.pump();

    expect(find.text('Connection to the Device was lost'), findsOneWidget);
    expect(find.text('Make sure it’s on and nearby.'), findsOneWidget);
    expect(find.text('München, Marienplatz'), findsNothing);

    await tester.tap(find.widgetWithText(FilledButton, 'Reconnect'));
    await tester.pump(const Duration(seconds: 1));
    await tester.pump();

    expect(link.connectCount, 2);
    expect(find.text('München, Marienplatz'), findsOneWidget);
  });

  testWidgets('a failed connection can be retried', (tester) async {
    final link = FakeDeviceLink(info: info, settings: settings)
      ..connectError = const DeviceLinkException(
        DeviceLinkError.connectionFailed,
      );
    await tester.pumpWidget(
      CompanionApp(link: link, providerFor: (_) => mvvProvider()),
    );
    link.emitDevices(const [_device]);
    await tester.pump();
    await tester.tap(find.text(_device.name));
    await tester.pump(const Duration(seconds: 1));
    await tester.pump();

    expect(find.text('Couldn’t connect to the Device'), findsOneWidget);
    expect(
      find.text(
        'Make sure it’s on and nearby, and that you entered the right passkey.',
      ),
      findsOneWidget,
    );

    link.connectError = null;
    await tester.tap(find.widgetWithText(FilledButton, 'Try again'));
    await tester.pump(const Duration(seconds: 1));
    await tester.pump();

    expect(link.disconnectCount, 1);
    expect(link.connectCount, 2);
    expect(find.text('München, Marienplatz'), findsOneWidget);
  });

  testWidgets('going back disconnects from the Device and scans again', (
    tester,
  ) async {
    final link = await openDevice(tester);

    await tester.pageBack();
    await tester.pump(const Duration(seconds: 1));

    expect(link.disconnectCount, 1);
    expect(link.scanCount, 2);
    expect(find.text('Looking for Devices…'), findsOneWidget);
  });

  Future<void> saveWifi(WidgetTester tester, String network) async {
    await tester.tap(find.widgetWithText(ListTile, 'WiFi network'));
    await tester.pump(const Duration(seconds: 1));
    await tester.pump(const Duration(seconds: 1));
    await tester.enterText(find.byType(TextFormField).at(0), network);
    await tester.tap(find.text('Save'));
    await tester.pump(const Duration(seconds: 1));
    await tester.pump(const Duration(seconds: 1));
  }

  Future<void> saveStation(
    WidgetTester tester,
    String query,
    String name,
  ) async {
    await tester.tap(find.widgetWithText(ListTile, 'Station'));
    await tester.pump(const Duration(seconds: 1));
    await tester.pump();
    await tester.enterText(find.byType(TextField), query);
    await tester.pump(const Duration(milliseconds: 400));
    await tester.pump();
    await tester.tap(find.text(name));
    await tester.pump();
    await tester.tap(find.text('Save'));
    await tester.pump(const Duration(seconds: 1));
    await tester.pump(const Duration(seconds: 1));
  }

  testWidgets(
    'saving a Station keeps the connection and shows the new Station',
    (tester) async {
      final link = await openDevice(tester);
      await saveStation(tester, 'Garchi', 'Garching');

      expect(link.writes.single.settings.stationId, 'de:09162:6');
      expect(find.text('The Device is restarting'), findsNothing);
      expect(link.connectCount, 1);
      expect(link.disconnectCount, 0);
      expect(find.text('Garching'), findsOneWidget);
      expect(find.text('All Route Directions'), findsOneWidget);
    },
  );

  testWidgets(
    'a failed re-read after saving a Station says the connection was lost',
    (tester) async {
      final link = await openDevice(tester);
      link.readError = _connectFailure;
      await saveStation(tester, 'Garchi', 'Garching');

      expect(find.text('Connection to the Device was lost'), findsOneWidget);
      expect(find.text('The Device is restarting'), findsNothing);
      expect(find.text('München, Marienplatz'), findsNothing);
    },
  );

  testWidgets('saving settings restarts the Device and reconnects with them', (
    tester,
  ) async {
    final link = await openDevice(tester);
    await saveWifi(tester, 'office');

    expect(find.text('The Device is restarting'), findsOneWidget);
    expect(find.widgetWithText(FilledButton, 'Back to Devices'), findsNothing);

    await tester.pump(_restartDelay);
    await tester.pump();

    expect(link.disconnectCount, 1);
    expect(link.connectCount, 2);
    expect(find.text('The Device is restarting'), findsNothing);
    expect(find.text('office'), findsOneWidget);
  });

  testWidgets('a restart retries until the Device is back', (tester) async {
    final link = await openDevice(tester);
    link.connectError = _connectFailure;
    await saveWifi(tester, 'office');

    await tester.pump(_restartDelay);
    await tester.pump(_restartDelay);

    expect(link.connectCount, 3);
    expect(find.text('The Device is restarting'), findsOneWidget);
    expect(find.byType(CircularProgressIndicator), findsOneWidget);

    link.connectError = null;
    await tester.pump(_restartDelay);
    await tester.pump();

    expect(link.connectCount, 4);
    expect(find.text('The Device is restarting'), findsNothing);
    expect(find.text('office'), findsOneWidget);
  });

  testWidgets(
    'when every restart attempt fails, the connection failure is shown',
    (tester) async {
      final link = await openDevice(tester);
      link.connectError = _connectFailure;
      await saveWifi(tester, 'office');

      for (var attempt = 0; attempt < 10; attempt++) {
        await tester.pump(_restartDelay);
      }
      await tester.pump();

      expect(link.connectCount, 11);
      expect(find.text('The Device is restarting'), findsNothing);
      expect(find.text('Couldn’t connect to the Device'), findsOneWidget);
    },
  );

  testWidgets('a connection that finishes after leaving the Device is closed', (
    tester,
  ) async {
    final link = FakeDeviceLink(info: info, settings: settings)
      ..connectGate = Completer<void>();
    await tester.pumpWidget(
      CompanionApp(link: link, providerFor: (_) => mvvProvider()),
    );
    link.emitDevices(const [_device]);
    await tester.pump();
    await tester.tap(find.text(_device.name));
    await tester.pump(const Duration(seconds: 1));
    await tester.pump();

    Navigator.of(tester.element(find.byType(DeviceScreen))).pop();
    await tester.pump(const Duration(seconds: 1));
    final disconnectsBeforeConnected = link.disconnectCount;

    link.connectGate!.complete();
    await tester.pump();
    await tester.pump();

    expect(link.disconnectCount, disconnectsBeforeConnected + 1);
  });

  testWidgets('leaving the Device while it restarts stops retrying', (
    tester,
  ) async {
    final link = await openDevice(tester);
    link.connectError = _connectFailure;
    await saveWifi(tester, 'office');

    await tester.pageBack();
    for (var step = 0; step < 4; step++) {
      await tester.pump(const Duration(milliseconds: 250));
    }
    expect(find.text('The Device is restarting'), findsNothing);

    await tester.pump(_restartDelay);

    expect(link.connectCount, 1);
  });
}
