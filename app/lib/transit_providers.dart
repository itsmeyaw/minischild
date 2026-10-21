import 'package:http/http.dart' as http;

import 'mvv_provider.dart';
import 'transit_provider.dart';

TransitProvider? transitProviderFor(String providerName, http.Client client) {
  return switch (providerName) {
    'mvv' => MvvProvider(client),
    _ => null,
  };
}
