import 'package:flutter/material.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:minischild/app.dart';
import 'package:minischild/ble_protocol.dart';
import 'package:minischild/device_link.dart';
import 'package:minischild/transit_provider.dart';

import 'fakes.dart';

const _device = FoundDevice(id: 'a', name: 'Minischild-1F2A');

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
}
