import 'package:flutter/material.dart';

import 'device_link.dart';
import 'device_list_screen.dart';
import 'transit_provider.dart';

const _seedColor = Color(0xFF1B5E9E);

class CompanionApp extends StatelessWidget {
  const CompanionApp({
    super.key,
    required this.link,
    required this.providerFor,
  });

  final DeviceLink link;
  final ProviderLookup providerFor;

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Minischild',
      theme: ThemeData(
        colorScheme: ColorScheme.fromSeed(seedColor: _seedColor),
      ),
      darkTheme: ThemeData(
        colorScheme: ColorScheme.fromSeed(
          seedColor: _seedColor,
          brightness: Brightness.dark,
        ),
      ),
      themeMode: ThemeMode.system,
      home: DeviceListScreen(link: link, providerFor: providerFor),
    );
  }
}
