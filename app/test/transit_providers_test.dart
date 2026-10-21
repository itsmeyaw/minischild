import 'package:flutter_test/flutter_test.dart';
import 'package:http/testing.dart';
import 'package:minischild/mvv_provider.dart';
import 'package:minischild/transit_providers.dart';

void main() {
  final client = MockClient((request) async => throw UnimplementedError());

  test('mvv names the MVV provider', () {
    expect(transitProviderFor('mvv', client), isA<MvvProvider>());
  });

  test('an unknown provider name has no provider', () {
    expect(transitProviderFor('bvg', client), isNull);
  });
}
