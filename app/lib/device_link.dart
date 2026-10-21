import 'ble_protocol.dart';

class FoundDevice {
  final String id;
  final String name;

  const FoundDevice({required this.id, required this.name});
}

enum DeviceLinkError { bluetoothOff, permissionDenied, connectionFailed }

class DeviceLinkException implements Exception {
  final DeviceLinkError error;

  const DeviceLinkException(this.error);
}

abstract interface class DeviceLink {
  /// Every Device found so far, re-emitted as the list grows. Errors with
  /// DeviceLinkException(bluetoothOff | permissionDenied). Cancelling stops scanning.
  Stream<List<FoundDevice>> scan();

  /// Connects to [device] and reads `info`. Throws DeviceLinkException(connectionFailed).
  Future<DeviceInfo> connect(FoundDevice device);

  /// Reads `settings` from the connected Device. Throws DeviceLinkException(connectionFailed).
  Future<Settings> readSettings();

  /// Emits when the open link drops.
  Stream<void> get connectionLost;

  Future<void> disconnect();
}
