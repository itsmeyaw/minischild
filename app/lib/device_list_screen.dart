import 'dart:async';

import 'package:flutter/material.dart';

import 'device_link.dart';
import 'status_message.dart';
import 'transit_provider.dart';

class DeviceListScreen extends StatefulWidget {
  const DeviceListScreen({
    super.key,
    required this.link,
    required this.providerFor,
  });

  final DeviceLink link;
  final TransitProvider? Function(String providerName) providerFor;

  @override
  State<DeviceListScreen> createState() => _DeviceListScreenState();
}

class _DeviceListScreenState extends State<DeviceListScreen> {
  StreamSubscription<List<FoundDevice>>? _scan;
  List<FoundDevice> _devices = const [];
  Object? _error;

  @override
  void initState() {
    super.initState();
    _startScan();
  }

  @override
  void dispose() {
    _scan?.cancel();
    super.dispose();
  }

  void _startScan() {
    _scan?.cancel();
    _scan = widget.link.scan().listen(
      (devices) => setState(() => _devices = devices),
      onError: (Object error) => setState(() => _error = error),
    );
  }

  void _retry() {
    setState(() {
      _devices = const [];
      _error = null;
    });
    _startScan();
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(title: const Text('Minischild')),
      body: _body(),
    );
  }

  Widget _body() {
    if (_error case final error?) return _scanFailure(error);
    if (_devices.isEmpty) {
      return const StatusMessage(
        icon: CircularProgressIndicator(),
        headline: 'Looking for Devices…',
        message: 'Keep your phone near the Device.',
      );
    }
    return ListView(
      children: [
        for (final device in _devices)
          ListTile(
            leading: const Icon(Icons.bluetooth),
            title: Text(device.name),
            trailing: const Icon(Icons.chevron_right),
          ),
      ],
    );
  }

  Widget _scanFailure(Object error) {
    final tryAgain = FilledButton(
      onPressed: _retry,
      child: const Text('Try again'),
    );
    return switch (error) {
      DeviceLinkException(error: DeviceLinkError.bluetoothOff) => StatusMessage(
        icon: const Icon(Icons.bluetooth_disabled, size: 48),
        headline: 'Bluetooth is off',
        message: 'Turn on Bluetooth to find your Device.',
        action: tryAgain,
      ),
      DeviceLinkException(error: DeviceLinkError.permissionDenied) =>
        StatusMessage(
          icon: const Icon(Icons.lock_outline, size: 48),
          headline: 'Bluetooth permission needed',
          message:
              'Allow Minischild to use Bluetooth in your phone’s Settings, then try again.',
          action: tryAgain,
        ),
      _ => StatusMessage(
        icon: const Icon(Icons.error_outline, size: 48),
        headline: 'Couldn’t look for Devices',
        message: 'Something went wrong while scanning.',
        action: tryAgain,
      ),
    };
  }
}
