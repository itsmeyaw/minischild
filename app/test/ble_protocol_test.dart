import 'dart:convert';
import 'dart:io';

import 'package:flutter_test/flutter_test.dart';
import 'package:minischild/ble_protocol.dart';

List<int> readExample(String name) =>
    File('../docs/ble-protocol/$name').readAsBytesSync();

void main() {
  test('decodes the info document', () {
    final info = DeviceInfo.decode(readExample('valid/info.json'));

    expect(info.provider, 'mvv');
    expect(info.firmware, 'v0.1.0');
  });

  test('decodes a settings read', () {
    final settings = Settings.decode(readExample('valid/settings-read.json'));

    expect(settings.stationId, 'de:09162:2');
    expect(settings.routeDirectionIds, [
      'ddb:920S1: :H:j26',
      'ddb:920S1: :R:j26',
    ]);
    expect(settings.platforms, ['1', '2']);
    expect(settings.wifiSsid, 'home');
  });

  test('decodes a settings write and ignores its password', () {
    final settings = Settings.decode(readExample('valid/settings-write.json'));

    expect(settings.stationId, 'de:09162:2');
    expect(settings.routeDirectionIds, [
      'ddb:920S1: :H:j26',
      'ddb:920S1: :R:j26',
    ]);
    expect(settings.platforms, ['1', '2']);
    expect(settings.wifiSsid, 'home');
  });

  test('decodes a settings write that keeps the stored password', () {
    final settings = Settings.decode(
      readExample('valid/settings-write-keep-password.json'),
    );

    expect(settings.stationId, 'de:09162:6');
    expect(settings.routeDirectionIds, isEmpty);
    expect(settings.platforms, isEmpty);
    expect(settings.wifiSsid, 'office');
  });

  test('encodes a settings write with the wifi password', () {
    final settings = Settings.decode(readExample('valid/settings-write.json'));

    expect(
      jsonDecode(utf8.decode(settings.encode(wifiPassword: 'correct horse'))),
      jsonDecode(utf8.decode(readExample('valid/settings-write.json'))),
    );
  });

  test('encodes a settings write that keeps the stored password', () {
    final settings = Settings.decode(
      readExample('valid/settings-write-keep-password.json'),
    );

    expect(
      jsonDecode(utf8.decode(settings.encode())),
      jsonDecode(
        utf8.decode(readExample('valid/settings-write-keep-password.json')),
      ),
    );
  });

  test('rejects malformed JSON', () {
    expect(
      () => Settings.decode(readExample('invalid/malformed.json')),
      throwsFormatException,
    );
  });

  test('rejects a document over 512 bytes', () {
    expect(
      () => Settings.decode(readExample('invalid/oversized.json')),
      throwsFormatException,
    );
  });

  test('rejects a document with a missing station', () {
    expect(
      () => Settings.decode(readExample('invalid/missing-station.json')),
      throwsFormatException,
    );
  });

  test('rejects a document with a wrong-typed field', () {
    expect(
      () => Settings.decode(readExample('invalid/wrong-type.json')),
      throwsFormatException,
    );
  });

  test('rejects an empty stationId', () {
    expect(
      () => Settings.decode(
        utf8.encode(
          '{"stationId": "", "routeDirectionIds": [], "platforms": [], "wifiSsid": "home"}',
        ),
      ),
      throwsFormatException,
    );
  });

  test('rejects a route direction id that is not a string', () {
    expect(
      () => Settings.decode(
        utf8.encode(
          '{"stationId": "de:09162:2", "routeDirectionIds": [1], "platforms": [], "wifiSsid": "home"}',
        ),
      ),
      throwsFormatException,
    );
  });

  test('rejects a wifiSsid over 32 UTF-8 bytes', () {
    final ssid = 'a' * 33;
    expect(
      () => Settings.decode(
        utf8.encode(
          '{"stationId": "de:09162:2", "routeDirectionIds": [], "platforms": [], "wifiSsid": "$ssid"}',
        ),
      ),
      throwsFormatException,
    );
  });
}
