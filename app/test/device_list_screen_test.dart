import 'package:flutter/material.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:minischild/app.dart';
import 'package:minischild/device_link.dart';

import 'fakes.dart';

void main() {
  final info = exampleInfo();
  final settings = exampleSettings('settings-read.json');

  testWidgets('looks for Devices, then lists the names of the ones found', (
    tester,
  ) async {
    final link = FakeDeviceLink(info: info, settings: settings);
    await tester.pumpWidget(CompanionApp(link: link, providerFor: (_) => null));

    expect(find.text('Looking for Devices…'), findsOneWidget);
    expect(find.text('Keep your phone near the Device.'), findsOneWidget);

    link.emitDevices(const [
      FoundDevice(id: 'a', name: 'Minischild-1F2A'),
      FoundDevice(id: 'b', name: 'Minischild-9C4D'),
    ]);
    await tester.pump();

    expect(find.text('Looking for Devices…'), findsNothing);
    expect(find.text('Minischild-1F2A'), findsOneWidget);
    expect(find.text('Minischild-9C4D'), findsOneWidget);
  });

  testWidgets('Bluetooth off explains it, and Try again scans again', (
    tester,
  ) async {
    final link = FakeDeviceLink(info: info, settings: settings);
    await tester.pumpWidget(CompanionApp(link: link, providerFor: (_) => null));

    link.failScan(DeviceLinkError.bluetoothOff);
    await tester.pump();

    expect(find.text('Bluetooth is off'), findsOneWidget);
    expect(find.text('Turn on Bluetooth to find your Device.'), findsOneWidget);

    await tester.tap(find.widgetWithText(FilledButton, 'Try again'));
    await tester.pump();

    expect(link.scanCount, 2);
    expect(find.text('Bluetooth is off'), findsNothing);
    expect(find.text('Looking for Devices…'), findsOneWidget);
  });

  testWidgets('a denied Bluetooth permission says what to allow', (
    tester,
  ) async {
    final link = FakeDeviceLink(info: info, settings: settings);
    await tester.pumpWidget(CompanionApp(link: link, providerFor: (_) => null));

    link.failScan(DeviceLinkError.permissionDenied);
    await tester.pump();

    expect(find.text('Bluetooth permission needed'), findsOneWidget);
    expect(
      find.text(
        'Allow Minischild to use Bluetooth in your phone’s Settings, then try again.',
      ),
      findsOneWidget,
    );
  });
}
