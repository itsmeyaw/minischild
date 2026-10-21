import 'dart:async';

import 'package:flutter/foundation.dart';
import 'package:universal_ble/universal_ble.dart';

import 'ble_protocol.dart';
import 'device_link.dart';

const _serviceUuid = 'd2c3f200-ea7c-4ae4-bc29-8b05a10aa4c7';
const _infoUuid = 'd2c3f201-ea7c-4ae4-bc29-8b05a10aa4c7';
const _settingsUuid = 'd2c3f202-ea7c-4ae4-bc29-8b05a10aa4c7';

// The first encrypted read waits for the user to type the passkey.
const _pairingTimeout = Duration(seconds: 60);

class UniversalBleDeviceLink implements DeviceLink {
  final _connectionLost = StreamController<void>.broadcast();
  StreamSubscription<bool>? _connectionSubscription;
  String? _deviceId;

  @override
  Stream<List<FoundDevice>> scan() {
    final found = <String, FoundDevice>{};
    StreamSubscription<BleDevice>? scanSubscription;
    var cancelled = false;
    late final StreamController<List<FoundDevice>> controller;

    controller = StreamController<List<FoundDevice>>(
      onListen: () async {
        scanSubscription = UniversalBle.scanStream.listen((device) {
          final name = device.name;
          if (name == null ||
              name.isEmpty ||
              found.containsKey(device.deviceId)) {
            return;
          }
          found[device.deviceId] = FoundDevice(id: device.deviceId, name: name);
          controller.add(
            found.values.toList()..sort((a, b) => a.name.compareTo(b.name)),
          );
        });
        try {
          final state = await UniversalBle.getBluetoothAvailabilityState();
          if (state == AvailabilityState.poweredOff) {
            throw const DeviceLinkException(DeviceLinkError.bluetoothOff);
          }
          if (state == AvailabilityState.unauthorized) {
            throw const DeviceLinkException(DeviceLinkError.permissionDenied);
          }
          try {
            await UniversalBle.requestPermissions();
          } catch (_) {
            throw const DeviceLinkException(DeviceLinkError.permissionDenied);
          }
          // Cancelled while waiting for the permission prompt.
          if (cancelled) return;
          await UniversalBle.startScan(
            scanFilter: ScanFilter(withServices: [_serviceUuid]),
          );
        } catch (error) {
          controller.addError(error);
        }
      },
      onCancel: () async {
        cancelled = true;
        await scanSubscription?.cancel();
        await UniversalBle.stopScan();
      },
    );

    return controller.stream;
  }

  @override
  Future<DeviceInfo> connect(FoundDevice device) async {
    try {
      _deviceId = device.id;
      await UniversalBle.connect(device.id);
      await UniversalBle.discoverServices(device.id);
      _connectionSubscription = UniversalBle.connectionStream(device.id)
          .where((connected) => !connected)
          .listen((_) => _connectionLost.add(null));
      final bytes = await UniversalBle.read(
        device.id,
        _serviceUuid,
        _infoUuid,
        timeout: _pairingTimeout,
      );
      return DeviceInfo.decode(bytes);
    } catch (error) {
      debugPrint('Connecting to ${device.name} failed: $error');
      await disconnect();
      throw const DeviceLinkException(DeviceLinkError.connectionFailed);
    }
  }

  @override
  Future<Settings> readSettings() async {
    try {
      final bytes = await UniversalBle.read(
        _deviceId!,
        _serviceUuid,
        _settingsUuid,
      );
      return Settings.decode(bytes);
    } catch (error) {
      debugPrint('Reading settings failed: $error');
      throw const DeviceLinkException(DeviceLinkError.connectionFailed);
    }
  }

  @override
  Future<void> writeSettings(
    Settings settings, {
    String wifiPassword = '',
  }) async {
    try {
      await UniversalBle.write(
        _deviceId!,
        _serviceUuid,
        _settingsUuid,
        Uint8List.fromList(settings.encode(wifiPassword: wifiPassword)),
      );
    } catch (error) {
      debugPrint('Writing settings failed: $error');
      throw const DeviceLinkException(DeviceLinkError.connectionFailed);
    }
  }

  @override
  Stream<void> get connectionLost => _connectionLost.stream;

  @override
  Future<void> disconnect() async {
    await _connectionSubscription?.cancel();
    _connectionSubscription = null;
    final deviceId = _deviceId;
    _deviceId = null;
    if (deviceId == null) return;
    try {
      await UniversalBle.disconnect(deviceId);
    } catch (_) {}
  }
}
