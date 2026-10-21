
# BVG Provider

Values come from the community [`transport.rest`](https://v6.bvg.transport.rest) API, which serves BVG and S-Bahn departures for Berlin without a key. It is unofficial and may change or disappear; see [ADR 0008](../../../../docs/adr/0008-bvg-transport-rest-endpoint.md).

The `STATION_ID` is the IBNR of the Station. To find it, search for the stop name:

```
https://v6.bvg.transport.rest/locations?query=Alexanderplatz&results=3&stops=true&addresses=false&poi=false
```

`S+U Alexanderplatz Bhf` is `900100003`.

The `ROUTE_DIRECTION_IDS` have the form `<line>|<destination IBNR>`, for example `U2|900130002` (U2 towards S+U Pankow). To find them, open the Station's departures and take `line.name` and `destination.id` from each entry:

```
https://v6.bvg.transport.rest/stops/900100003/departures
```

`PLATFORMS` match the `platform` text exactly as the API names it, for example `Pos. 15` or `2 (U2)`.

```cpp
constexpr const char* STATION_ID = "900100003";
inline const std::vector<std::string> ROUTE_DIRECTION_IDS = {"U2|900130002"};
```

Build with:

```sh
PROVIDER=bvg pio run -e esp32dev
```

The same code works for Berlin-Brandenburg: change the host in `departures_url.h` to `v6.vbb.transport.rest` and use the IDs from that host.
