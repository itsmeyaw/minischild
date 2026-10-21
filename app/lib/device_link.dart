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

  /// Writes [settings] to the connected Device. It restarts, dropping the link,
  /// only if the WiFi network or password changed. An empty [wifiPassword] keeps
  /// the stored one. Throws FormatException, writing nothing, if the document is
  /// over 512 bytes, else DeviceLinkException(connectionFailed).
  Future<void> writeSettings(Settings settings, {String wifiPassword = ''});

  /// Emits when the open link drops.
  Stream<void> get connectionLost;

  Future<void> disconnect();
}
