# Use the transport.rest HAFAS API for BVG departures

Departures for the BVG provider come from the community `v6.bvg.transport.rest` API, a keyless HAFAS REST wrapper covering Berlin, rather than an official VBB or BVG API, which needs a key. Its `/stops/<IBNR>/departures` endpoint returns JSON that the Device stream-parses: each Departure is about 1.8 KB, so `results=20` is read through an ArduinoJson filter that keeps only the fields a Departure needs. The API accepts a single `direction` IBNR, so the Device filters Route Directions itself, matching each Departure's `<line.name>|<destination.id>` against `ROUTE_DIRECTION_IDS` in `config.h`. Cancelled Departures are dropped. TLS is pinned to the ISRG Root X2 certificate, which expires on 2040-09-17.

## Considered Options

- **Official VBB or BVG API**: supported and documented, but needs an API key that the Device would have to carry.
- **`v6.db.transport.rest`**: the same HAFAS wrapper under its earlier host, now retired; it returns 503.
- **GTFS Realtime feeds**: too large for an ESP32 without PSRAM, as in [ADR 0001](0001-mvv-website-endpoints.md).

## Consequences

The host is a community service with no SLA. Outages were seen in August and October 2026, and its maintainers point users to Transitous. All knowledge of the endpoint stays in the BVG `DepartureProvider`, so switching to another source touches only that folder. The same code serves Berlin-Brandenburg by changing the host to `v6.vbb.transport.rest`.

This change does not add a `bvg` `TransitProvider` to the Companion App. For a provider name it doesn't know, the app only allows editing WiFi ([ADR 0007](0007-provider-agnostic-platform-filter.md)), so a BVG Board Selection is set in `config.h` until a follow-up adds the app side.
