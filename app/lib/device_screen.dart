import 'dart:async';

import 'package:flutter/material.dart';

import 'ble_protocol.dart';
import 'device_link.dart';
import 'status_message.dart';
import 'transit_provider.dart';

class DeviceScreen extends StatefulWidget {
  const DeviceScreen({
    super.key,
    required this.device,
    required this.link,
    required this.providerFor,
  });

  final FoundDevice device;
  final DeviceLink link;
  final ProviderLookup providerFor;

  @override
  State<DeviceScreen> createState() => _DeviceScreenState();
}

sealed class _Connection {
  const _Connection();
}

class _Connecting extends _Connection {
  const _Connecting();
}

class _ConnectFailed extends _Connection {
  const _ConnectFailed();
}

class _ConnectionLost extends _Connection {
  const _ConnectionLost();
}

class _Connected extends _Connection {
  const _Connected(this.details);

  final _DeviceDetails details;
}

sealed class _Names {
  const _Names();
}

class _UnsupportedProvider extends _Names {
  const _UnsupportedProvider();
}

class _NamesUnavailable extends _Names {
  const _NamesUnavailable();
}

class _NamesFound extends _Names {
  const _NamesFound(this.stationName, this.routeLabels);

  final String? stationName;
  final Map<String, String> routeLabels;
}

class _DeviceDetails {
  const _DeviceDetails({
    required this.info,
    required this.settings,
    required this.names,
  });

  final DeviceInfo info;
  final Settings settings;
  final _Names names;
}

class _DeviceScreenState extends State<DeviceScreen> {
  _Connection _connection = const _Connecting();
  StreamSubscription<void>? _connectionLostSubscription;

  @override
  void initState() {
    super.initState();
    _connectionLostSubscription = widget.link.connectionLost.listen((_) {
      if (_connection is _Connected) {
        setState(() => _connection = const _ConnectionLost());
      }
    });
    _connect();
  }

  @override
  void dispose() {
    _connectionLostSubscription?.cancel();
    unawaited(widget.link.disconnect());
    super.dispose();
  }

  Future<void> _reconnect() async {
    setState(() => _connection = const _Connecting());
    await widget.link.disconnect();
    await _connect();
  }

  Future<void> _connect() async {
    try {
      final info = await widget.link.connect(widget.device);
      final settings = await widget.link.readSettings();
      final names = await _lookUp(info, settings);
      if (mounted) {
        setState(
          () => _connection = _Connected(
            _DeviceDetails(info: info, settings: settings, names: names),
          ),
        );
      }
    } catch (error) {
      debugPrint('Connecting to ${widget.device.name} failed: $error');
      if (mounted) setState(() => _connection = const _ConnectFailed());
    }
  }

  Future<_Names> _lookUp(DeviceInfo info, Settings settings) async {
    final provider = widget.providerFor(info.provider);
    if (provider == null) return const _UnsupportedProvider();
    final stationId = settings.stationId;
    try {
      final (stations, routes) = await (
        provider.searchStations(stationId),
        provider.routeDirections(stationId),
      ).wait.timeout(const Duration(seconds: 10));
      return _NamesFound(
        stations.where((station) => station.id == stationId).firstOrNull?.name,
        {
          for (final route in routes)
            route.id: '${route.route} towards ${route.headsign}',
        },
      );
    } catch (error) {
      debugPrint('Looking up names for ${widget.device.name} failed: $error');
      return const _NamesUnavailable();
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(title: Text(widget.device.name)),
      body: _body(),
    );
  }

  Widget _body() {
    return switch (_connection) {
      _Connecting() => const StatusMessage(
        icon: CircularProgressIndicator(),
        headline: 'Connecting…',
        message: 'The first time, your phone asks for the Device’s passkey.',
      ),
      _ConnectFailed() => _retryMessage(
        headline: 'Couldn’t connect to the Device',
        message:
            'Make sure it’s on and nearby, and that you entered the right passkey.',
        action: 'Try again',
      ),
      _ConnectionLost() => _retryMessage(
        headline: 'Connection to the Device was lost',
        message: 'Make sure it’s on and nearby.',
        action: 'Reconnect',
      ),
      _Connected(:final details) => _loaded(details),
    };
  }

  Widget _retryMessage({
    required String headline,
    required String message,
    required String action,
  }) {
    return StatusMessage(
      icon: const Icon(Icons.bluetooth_disabled, size: 48),
      headline: headline,
      message: message,
      action: FilledButton(onPressed: _reconnect, child: Text(action)),
    );
  }

  Widget _loaded(_DeviceDetails details) {
    return ListView(
      children: [
        const _SectionHeader('Board Selection'),
        ..._selection(details),
        const _SectionHeader('Device'),
        ListTile(
          title: const Text('WiFi network'),
          subtitle: Text(details.settings.wifiSsid),
        ),
        ListTile(
          title: const Text('Provider'),
          subtitle: Text(details.info.provider),
        ),
        ListTile(
          title: const Text('Firmware'),
          subtitle: Text(details.info.firmware),
        ),
      ],
    );
  }

  List<Widget> _selection(_DeviceDetails details) {
    final settings = details.settings;
    return switch (details.names) {
      _UnsupportedProvider() => [
        ListTile(
          leading: const Icon(Icons.info_outline),
          title: const Text('Unsupported provider'),
          subtitle: Text(
            'This app can’t show the Board Selection for the “${details.info.provider}” provider. You can still see the WiFi network.',
          ),
        ),
      ],
      _NamesUnavailable() => [
        Padding(
          padding: const EdgeInsets.symmetric(horizontal: 16),
          child: Text(
            'Couldn’t reach the provider, so IDs are shown.',
            style: Theme.of(context).textTheme.bodySmall,
          ),
        ),
        ..._stationAndRoutes(settings, null, const {}),
      ],
      _NamesFound(:final stationName, :final routeLabels) => _stationAndRoutes(
        settings,
        stationName,
        routeLabels,
      ),
    };
  }

  List<Widget> _stationAndRoutes(
    Settings settings,
    String? stationName,
    Map<String, String> routeLabels,
  ) {
    return [
      ListTile(
        title: Text(stationName ?? settings.stationId),
        subtitle: const Text('Station'),
      ),
      const _SectionHeader('Route Directions'),
      if (settings.routeDirectionIds.isEmpty)
        const ListTile(title: Text('All Route Directions'))
      else
        for (final id in settings.routeDirectionIds)
          ListTile(title: Text(routeLabels[id] ?? id)),
      const _SectionHeader('Platforms'),
      if (settings.platforms.isEmpty)
        const ListTile(title: Text('All Platforms'))
      else
        for (final platform in settings.platforms)
          ListTile(title: Text('Platform $platform')),
    ];
  }
}

class _SectionHeader extends StatelessWidget {
  const _SectionHeader(this.title);

  final String title;

  @override
  Widget build(BuildContext context) {
    final theme = Theme.of(context);
    return Padding(
      padding: const EdgeInsets.fromLTRB(16, 24, 16, 8),
      child: Text(
        title,
        style: theme.textTheme.titleSmall?.copyWith(
          color: theme.colorScheme.primary,
        ),
      ),
    );
  }
}
