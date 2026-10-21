# BLE protocol

The GATT contract between a Device and the Companion App. Both sides implement it, and both sides' tests load the example documents in [`ble-protocol/`](ble-protocol). Change this file, the examples and both implementations together (ADR 0005).

## Advertising and pairing

- The Device advertises at all times as `Minischild-XXXX`, where `XXXX` is the last two bytes of its Bluetooth address in upper-case hex, and includes the service UUID in its advertisement.
- Both characteristics require an encrypted, authenticated link (LE Secure Connections with MITM protection). Unpaired access fails with an insufficient-authentication error, which makes the phone start pairing.
- Pairing uses the fixed 6-digit passkey `BLE_PASSKEY` from `config.h`: the Device acts as display-only, so the phone asks for it. Up to 3 phones stay bonded (ADR 0006).

## Service

| Name | UUID | Properties |
|---|---|---|
| Service | `d2c3f200-ea7c-4ae4-bc29-8b05a10aa4c7` | |
| `info` | `d2c3f201-ea7c-4ae4-bc29-8b05a10aa4c7` | read |
| `settings` | `d2c3f202-ea7c-4ae4-bc29-8b05a10aa4c7` | read, write (with response) |

Both values are UTF-8 JSON objects of at most **512 bytes**, the ATT attribute limit. There is no chunking beyond what ATT long reads and long writes already do.

## `info`

```json
{"provider": "mvv", "firmware": "v0.1.0"}
```

| Field | Type | Meaning |
|---|---|---|
| `provider` | string | The provider the firmware was built with (`PROVIDER`, ADR 0003). The Companion App picks its `TransitProvider` by this name (ADR 0007). |
| `firmware` | string | The firmware version (`git describe` at build time). |

## `settings`

The Device's Board Selection and WiFi network.

| Field | Type | Read | Write |
|---|---|---|---|
| `stationId` | non-empty string | always | required |
| `routeDirectionIds` | array of strings, `[]` = all | always | required |
| `platforms` | array of strings, `[]` = all | always | required |
| `wifiSsid` | string of 1–32 bytes | always | required |
| `wifiPassword` | string of at most 64 bytes | **never** | optional |

- A read never contains `wifiPassword`.
- A write with `wifiPassword` absent or `""` keeps the stored password, so the Board Selection can change without re-entering it.
- Unknown fields are ignored.
- With nothing written yet, reads return the values from `config.h`.

### Write behaviour

- **Valid write:** the Device saves the settings to non-volatile storage. If `wifiSsid` or `wifiPassword` changed, it restarts about a second later, dropping the BLE link, then shows the new Departure Board on the new network. Otherwise it keeps the link and shows the new Board Selection within about a second, fetching right away. Stored settings survive power loss and override `config.h`.
- **Invalid write** (not JSON, over 512 bytes, a required field missing or of the wrong type, a string out of range): the Device keeps its settings and doesn't apply or restart. The write itself still succeeds at the ATT level, so a client that needs to know reads `settings` back.

## Examples

- Valid: [`info.json`](ble-protocol/valid/info.json), [`settings-read.json`](ble-protocol/valid/settings-read.json) (what a read of the Device that received `settings-write.json` returns), [`settings-write.json`](ble-protocol/valid/settings-write.json), [`settings-write-keep-password.json`](ble-protocol/valid/settings-write-keep-password.json) (keeps the stored password, shows every Route Direction and Platform).
- Invalid: [`malformed.json`](ble-protocol/invalid/malformed.json), [`oversized.json`](ble-protocol/invalid/oversized.json), [`missing-station.json`](ble-protocol/invalid/missing-station.json), [`wrong-type.json`](ble-protocol/invalid/wrong-type.json).
