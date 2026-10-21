import 'dart:async';
import 'dart:io';

import 'package:minischild/ble_protocol.dart';
import 'package:minischild/device_link.dart';
import 'package:minischild/transit_provider.dart';

Settings exampleSettings(String name) =>
    Settings.decode(File('../docs/ble-protocol/valid/$name').readAsBytesSync());

DeviceInfo exampleInfo() => DeviceInfo.decode(
  File('../docs/ble-protocol/valid/info.json').readAsBytesSync(),
);

class FakeDeviceLink implements DeviceLink {
  FakeDeviceLink({required this.info, required this.settings});

  final DeviceInfo info;
  Settings settings;
  final _scans = <StreamController<List<FoundDevice>>>[];
  final _connectionLost = StreamController<void>.broadcast();
  int scanCount = 0;
  int connectCount = 0;
  int disconnectCount = 0;
  DeviceLinkException? connectError;
  Completer<void>? connectGate;
  final writes = <({Settings settings, String wifiPassword})>[];
  DeviceLinkException? writeError;
  DeviceLinkException? readError;

  @override
  Stream<List<FoundDevice>> scan() {
    scanCount++;
    final scan = StreamController<List<FoundDevice>>();
    _scans.add(scan);
    return scan.stream;
  }

  void emitDevices(List<FoundDevice> devices) => _scans.last.add(devices);

  void failScan(DeviceLinkError error) =>
      _scans.last.addError(DeviceLinkException(error));

  @override
  Future<DeviceInfo> connect(FoundDevice device) async {
    connectCount++;
    await connectGate?.future;
    if (connectError case final error?) throw error;
    return info;
  }

  @override
  Future<Settings> readSettings() async {
    if (readError case final error?) throw error;
    return settings;
  }

  @override
  Future<void> writeSettings(
    Settings settings, {
    String wifiPassword = '',
  }) async {
    settings.encode(wifiPassword: wifiPassword);
    if (writeError case final error?) throw error;
    writes.add((settings: settings, wifiPassword: wifiPassword));
    this.settings = settings;
  }

  @override
  Stream<void> get connectionLost => _connectionLost.stream;

  void loseConnection() => _connectionLost.add(null);

  @override
  Future<void> disconnect() async => disconnectCount++;
}

class FakeTransitProvider implements TransitProvider {
  FakeTransitProvider({
    this.stationResults = const [],
    this.routeResults = const [],
    this.error,
  });

  final List<Station> stationResults;
  final List<RouteDirection> routeResults;
  Object? error;
  int calls = 0;
  final queries = <String>[];

  @override
  Future<List<Station>> searchStations(String query) async {
    calls++;
    queries.add(query);
    if (error case final error?) throw error;
    return stationResults;
  }

  @override
  Future<List<RouteDirection>> routeDirections(String stationId) async {
    calls++;
    if (error case final error?) throw error;
    return routeResults;
  }
}
