import 'package:flutter/material.dart';
import 'package:http/http.dart' as http;

import 'app.dart';
import 'transit_providers.dart';
import 'universal_ble_device_link.dart';

void main() {
  final client = http.Client();
  runApp(
    CompanionApp(
      link: UniversalBleDeviceLink(),
      providerFor: (name) => transitProviderFor(name, client),
    ),
  );
}
