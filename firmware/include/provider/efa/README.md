# EFA Providers

The `vvs`, `vrr`, `kvv` and `bavaria` providers read departures from EFA (Elektronische Fahrplanauskunft) servers, the public journey-planner backend that several German regions run. The request, parsing, cancellation filter and TLS pinning are shared in this folder; each region is a provider folder under `firmware/src/provider/<name>/` whose `setup.cpp` names its server. See [ADR 0009](../../../../docs/adr/0009-efa-regional-providers.md).

The endpoints are unofficial and undocumented, may change without notice, and need no key.

| Provider | Region | Base URL |
|----------|--------|----------|
| `vvs` | VVS, Stuttgart | `https://www3.vvs.de/mngvvs` |
| `vrr` | VRR, Rhein-Ruhr | `https://efa.vrr.de/standard` |
| `kvv` | KVV, Karlsruhe | `https://projekte.kvv-efa.de/sl3-alone` |
| `bavaria` | Bahnland Bayern, including Munich | `https://www.bahnland-bayern.de/efa` |

The `STATION_ID` is the global ID of the Station. To find it, search for the stop name (the `<base>` is the region's base URL from the table):

```
<base>/XML_STOPFINDER_REQUEST?outputFormat=rapidJSON&type_sf=any&name_sf=Stuttgart%20Hbf&anyObjFilter_sf=2
```

Take `id` from the `locations` entry with `type` `stop`. On `vvs`, `Stuttgart Hbf` resolves to `de:08111:6118` (Hauptbahnhof (tief)).

The `ROUTE_DIRECTION_IDS` have the form `<line>|<destination ID>`, the same shape as the BVG provider. To find them, open the Station's departures and take `transportation.disassembledName` (or `transportation.number` when that is empty) and `transportation.destination.id` from each entry:

```
<base>/XML_DM_REQUEST?outputFormat=rapidJSON&type_dm=stop&name_dm=de%3A08111%3A6118&mode=direct&useRealtime=1&limit=20
```

On `vvs`, `U15|5006128` and `MEX18|5021000` are examples.

`PLATFORMS` match the platform text as the API names it, taken from `platformName` (falling back to `plannedPlatformName`, then `platform`), for example `Gleis 101`, `Gleis 1(U)` or `9`.

```cpp
constexpr const char* STATION_ID = "de:08111:6118";
inline const std::vector<std::string> ROUTE_DIRECTION_IDS = {"U15|5006128"};
```

Build with:

```sh
PROVIDER=vvs pio run -e esp32dev
```

Each request asks for 10 Departures, or 20 when Route Directions are selected, because the server has no usable line filter and the Device filters them. Cancelled Departures are dropped. Times are read as ISO 8601 UTC.

## Adding a region

1. Create `firmware/src/provider/<name>/setup.cpp` (about ten lines). It defines `setupDepartureProvider` and names an `EfaInstance` with the region's base URL and pinned root CA, as the existing region folders do.
2. Check that the server's certificate chains to the pinned root. If it chains to another root, add that root as a header next to `digicert_global_root_g2.h` and use it as `rootCa`.
3. Build with `PROVIDER=<name> pio run -e esp32dev`.

The shared code needs no change.
