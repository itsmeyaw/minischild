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

bool _isWifiSsid(String ssid) {
  final length = utf8.encode(ssid).length;
  return length >= 1 && length <= 32;
}

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

  factory Settings.decode(List<int> bytes) {
    return switch (_decodeObject(bytes)) {
      {
        'stationId': String stationId,
        'routeDirectionIds': List<Object?> routeDirectionIds,
        'platforms': List<Object?> platforms,
        'wifiSsid': String wifiSsid,
      }
          when stationId.isNotEmpty &&
              _isWifiSsid(wifiSsid) &&
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
}
