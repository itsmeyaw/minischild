import 'dart:convert';

const _maxBytes = 512;

Map<String, Object?> _decodeObject(List<int> bytes) {
  if (bytes.length > _maxBytes) {
    throw const FormatException('document is over 512 bytes');
  }
  final json = jsonDecode(utf8.decode(bytes));
  if (json is! Map<String, Object?>) {
    throw const FormatException('document is not a JSON object');
  }
  return json;
}

bool isWifiSsid(String ssid) {
  final length = utf8.encode(ssid).length;
  return length >= 1 && length <= 32;
}

bool isWifiPassword(String password) => utf8.encode(password).length <= 64;

class DeviceInfo {
  final String provider;
  final String firmware;

  const DeviceInfo({required this.provider, required this.firmware});

  factory DeviceInfo.decode(List<int> bytes) {
    return switch (_decodeObject(bytes)) {
      {'provider': String provider, 'firmware': String firmware} => DeviceInfo(
        provider: provider,
        firmware: firmware,
      ),
      _ => throw const FormatException(
        'info needs string provider and firmware',
      ),
    };
  }
}

class Settings {
  final String stationId;
  final List<String> routeDirectionIds;
  final List<String> platforms;
  final String wifiSsid;

  const Settings({
    required this.stationId,
    required this.routeDirectionIds,
    required this.platforms,
    required this.wifiSsid,
  });

  Settings copyWith({
    String? stationId,
    List<String>? routeDirectionIds,
    List<String>? platforms,
    String? wifiSsid,
  }) {
    return Settings(
      stationId: stationId ?? this.stationId,
      routeDirectionIds: routeDirectionIds ?? this.routeDirectionIds,
      platforms: platforms ?? this.platforms,
      wifiSsid: wifiSsid ?? this.wifiSsid,
    );
  }

  factory Settings.decode(List<int> bytes) {
    return switch (_decodeObject(bytes)) {
      {
        'stationId': String stationId,
        'routeDirectionIds': List<Object?> routeDirectionIds,
        'platforms': List<Object?> platforms,
        'wifiSsid': String wifiSsid,
      }
          when stationId.isNotEmpty &&
              isWifiSsid(wifiSsid) &&
              routeDirectionIds.every((id) => id is String) &&
              platforms.every((platform) => platform is String) =>
        Settings(
          stationId: stationId,
          routeDirectionIds: List<String>.from(routeDirectionIds),
          platforms: List<String>.from(platforms),
          wifiSsid: wifiSsid,
        ),
      _ => throw const FormatException('settings are invalid'),
    };
  }

  /// Throws FormatException if the document is over 512 bytes.
  List<int> encode({String wifiPassword = ''}) {
    final bytes = utf8.encode(
      jsonEncode({
        'stationId': stationId,
        'routeDirectionIds': routeDirectionIds,
        'platforms': platforms,
        'wifiSsid': wifiSsid,
        if (wifiPassword.isNotEmpty) 'wifiPassword': wifiPassword,
      }),
    );
    if (bytes.length > _maxBytes) {
      throw const FormatException('document is over 512 bytes');
    }
    return bytes;
  }
}
