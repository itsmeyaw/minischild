# Share one EFA core across regional providers

Departures for `vvs`, `vrr`, `kvv` and `bavaria` come from EFA servers, which answer the same `XML_DM_REQUEST` and `XML_STOPFINDER_REQUEST` schema in each region. The request building, parsing, cancellation filter and TLS pinning live once in `firmware/include/provider/efa/` as header-only code. Each region is its own folder, `firmware/src/provider/<name>/`, whose `setup.cpp` names an `EfaInstance` with its base URL and pinned root CA. [ADR 0003](0003-compile-time-view-and-provider-selection.md) links only the picked provider folder, so `PROVIDER=vvs` builds Stuttgart without linking the other three regions. Each region keeps a distinct provider name, so the Companion App can tell the regions apart and search stations on the right server ([ADR 0007](0007-provider-agnostic-platform-filter.md)). Route Direction IDs use the `<line>|<destination ID>` shape from [ADR 0008](0008-bvg-transport-rest-endpoint.md), shared through `provider/route_direction.h`. Requests ask for 10 Departures, or 20 when Route Directions are selected, and cancelled Departures are dropped.

## Considered Options

- **One `efa` provider with a base URL chosen at compile or run time**: the provider name then no longer identifies the region, so the Companion App cannot tell regions apart. A run-time choice also links every region's TLS root, which ADR 0003 rejects.
- **A separate parser and request code per region**: the schema is identical on all four servers, so each fix would have to be made four times.

## Consequences

The servers are public, unofficial and undocumented, with no SLA, and may change without notice. A change to the schema is fixed once in the shared `efa` folder. TLS is pinned to DigiCert Global Root G2 (RSA, SHA-256 fingerprint `CB:3C:CB:B7:60:31:E5:E0:13:8F:8D:D3:9A:23:F9:DE:47:FF:C3:5E:43:C1:14:4C:EA:27:D4:6A:5A:B1:CB:5F`), which all four hosts were verified against with `openssl` and which expires on 2038-01-15. A server on another chain needs its own root header. The header-only provider class is compiled into the build of the picked region only.

Each Departure is about 1.5 to 12 KB, so 10 Departures are roughly 24 to 60 KB. They are read through an ArduinoJson filter that keeps only the fields a Departure needs. The request asks for 20 when filtering, because the servers have no usable line filter.

Other EFA servers (VRN, DING, EFA-BW) answered the same schema during exploration but are not included. Adding one means a new provider folder with its own `EfaInstance`.

This change does not add `TransitProvider`s for these regions to the Companion App. For a provider name it doesn't know, the app only allows editing WiFi ([ADR 0007](0007-provider-agnostic-platform-filter.md)), so their Board Selections are set in `config.h` until a follow-up adds the app side.
