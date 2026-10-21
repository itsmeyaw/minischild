import 'package:flutter/material.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:minischild/app.dart';
import 'package:minischild/ble_protocol.dart';
import 'package:minischild/device_link.dart';
import 'package:minischild/transit_provider.dart';
import 'package:minischild/wifi_screen.dart';

import 'fakes.dart';

const _device = FoundDevice(id: 'a', name: 'Minischild-1F2A');

void main() {
  final info = exampleInfo();
  final settings = exampleSettings('settings-read.json');
  FakeTransitProvider mvvProvider() => FakeTransitProvider(
    stationResults: const [
      Station(id: 'de:09162:2', name: 'München, Marienplatz'),
    ],
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
    DeviceInfo? deviceInfo,
    Settings? deviceSettings,
    ProviderLookup? providerFor,
  }) async {
    final provider = mvvProvider();
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

  Future<void> openWifi(WidgetTester tester) async {
    await tester.tap(find.widgetWithText(ListTile, 'WiFi network'));
    await tester.pump(const Duration(seconds: 1));
    await tester.pump(const Duration(seconds: 1));
  }

  Future<void> save(WidgetTester tester) async {
    await tester.tap(find.text('Save'));
    await tester.pump(const Duration(seconds: 1));
    await tester.pump(const Duration(seconds: 1));
  }

  Finder nameField() => find.byType(TextFormField).at(0);
  Finder passwordField() => find.byType(TextFormField).at(1);

  EditableText editable(WidgetTester tester, Finder field) =>
      tester.widget<EditableText>(
        find.descendant(of: field, matching: find.byType(EditableText)),
      );

  testWidgets(
    'opening WiFi shows the current network and a hidden, empty password',
    (tester) async {
      await openDevice(tester);
      await openWifi(tester);

      expect(editable(tester, nameField()).controller.text, 'home');
      expect(editable(tester, passwordField()).controller.text, isEmpty);
      expect(editable(tester, passwordField()).obscureText, isTrue);
    },
  );

  testWidgets('saving a new network and password restarts the Device', (
    tester,
  ) async {
    final link = await openDevice(tester);
    await openWifi(tester);

    await tester.enterText(nameField(), 'office');
    await tester.enterText(passwordField(), 'correct horse');
    await save(tester);

    expect(link.writes, hasLength(1));
    final write = link.writes.single;
    expect(write.settings.wifiSsid, 'office');
    expect(write.settings.stationId, settings.stationId);
    expect(write.settings.routeDirectionIds, settings.routeDirectionIds);
    expect(write.settings.platforms, settings.platforms);
    expect(write.wifiPassword, 'correct horse');
    expect(find.text('The Device is restarting'), findsOneWidget);
    await tester.pump(const Duration(seconds: 3));
    await tester.pump();
    expect(find.text('The Device is restarting'), findsNothing);
  });

  testWidgets('leaving the password empty keeps the stored one', (
    tester,
  ) async {
    final link = await openDevice(tester);
    await openWifi(tester);

    await tester.enterText(nameField(), 'office');
    await save(tester);

    expect(link.writes.single.settings.wifiSsid, 'office');
    expect(link.writes.single.wifiPassword, '');
    await tester.pump(const Duration(seconds: 3));
    await tester.pump();
  });

  testWidgets('an empty network name is refused and nothing is written', (
    tester,
  ) async {
    final link = await openDevice(tester);
    await openWifi(tester);

    await tester.enterText(nameField(), '');
    await save(tester);

    expect(find.text('Enter the network name.'), findsOneWidget);
    expect(link.writes, isEmpty);
  });

  testWidgets('a network name over 32 bytes is refused', (tester) async {
    final link = await openDevice(tester);
    await openWifi(tester);

    await tester.enterText(nameField(), 'a' * 33);
    await save(tester);

    expect(find.text('Network names are at most 32 bytes.'), findsOneWidget);
    expect(link.writes, isEmpty);
  });

  testWidgets('a password over 64 bytes is refused', (tester) async {
    final link = await openDevice(tester);
    await openWifi(tester);

    await tester.enterText(passwordField(), 'p' * 65);
    await save(tester);

    expect(find.text('Passwords are at most 64 bytes.'), findsOneWidget);
    expect(link.writes, isEmpty);
  });

  testWidgets(
    'a Device with an unsupported provider can still change its WiFi network',
    (tester) async {
      final link = await openDevice(
        tester,
        deviceInfo: const DeviceInfo(provider: 'bvg', firmware: 'v0.1.0'),
        providerFor: (_) => null,
      );
      await openWifi(tester);

      await tester.enterText(nameField(), 'office');
      await tester.enterText(passwordField(), 'correct horse');
      await save(tester);

      expect(link.writes.single.settings.wifiSsid, 'office');
      expect(find.text('The Device is restarting'), findsOneWidget);
      await tester.pump(const Duration(seconds: 3));
      await tester.pump();
      expect(find.text('The Device is restarting'), findsNothing);
    },
  );

  testWidgets('a failed save says so and keeps the WiFi screen open', (
    tester,
  ) async {
    final link = await openDevice(tester);
    link.writeError = const DeviceLinkException(
      DeviceLinkError.connectionFailed,
    );
    await openWifi(tester);

    await tester.enterText(nameField(), 'office');
    await save(tester);

    expect(
      find.descendant(
        of: find.byType(WifiScreen),
        matching: find.text(
          'Couldn’t save. Make sure the Device is on and nearby, then try again.',
        ),
      ),
      findsOneWidget,
    );
    expect(find.widgetWithText(AppBar, 'WiFi network'), findsOneWidget);
    expect(find.text('The Device is restarting'), findsNothing);
    expect(link.writes, isEmpty);
  });

  testWidgets('a document over 512 bytes is refused and nothing is written', (
    tester,
  ) async {
    final routeId = 'r' * 389;
    final deviceSettings = settings.copyWith(routeDirectionIds: [routeId]);
    expect(deviceSettings.encode().length, lessThanOrEqualTo(512));
    expect(
      () => deviceSettings.encode(wifiPassword: 'p' * 64),
      throwsFormatException,
    );
    final link = await openDevice(
      tester,
      deviceSettings: deviceSettings,
      providerFor: (name) => name == 'mvv'
          ? FakeTransitProvider(
              routeResults: [
                RouteDirection(id: routeId, route: 'S1', headsign: 'Freising'),
              ],
            )
          : null,
    );
    await openWifi(tester);

    await tester.enterText(passwordField(), 'p' * 64);
    await save(tester);

    expect(
      find.descendant(
        of: find.byType(WifiScreen),
        matching: find.text(
          'This is too much for the Device to store. Choose fewer Route Directions or Platforms, or use a shorter password.',
        ),
      ),
      findsOneWidget,
    );
    expect(find.text('The Device is restarting'), findsNothing);
    expect(link.writes, isEmpty);
  });
}
