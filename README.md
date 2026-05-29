# Remote Possibility — M5 Cardputer universal remote

**Hacker Planet LLC** · Philadelphia, PA · Founder **Salvador Data** ([salvadorData@proton.me](mailto:salvadorData@proton.me))

| | |
|---|---|
| GitHub | [salvador-Data/Remote-Possibility](https://github.com/salvador-Data/Remote-Possibility) |
| Reddit | [u/SalvadorData](https://www.reddit.com/user/SalvadorData) |
| Product | [hackerplanet.dev/cardputer.html#remote-possibility](https://hackerplanet.dev/cardputer.html#remote-possibility) |

Pocket **universal remote** for the M5Stack Cardputer: **infrared**, **433/315 MHz RF OOK**, and **CC1101 sub-GHz** (extensible SD profile format for any supported protocol). Learn OEM remotes, scan TV/fan/AC NEC libraries, save to microSD.

| SKU | Stripe key | Retail |
|-----|------------|--------|
| Remote Possibility | `remotePossibility` | **$99.99** (+ tax/shipping) |

## Features

- **IR** — NEC + raw via 38 kHz module (GPIO 44 TX / 1 RX)
- **RF** — 433 MHz OOK learn/send (RCSwitch); 315 MHz = swap module, same pins
- **Sub-GHz** — CC1101 SPI wiring documented; enable `RP_ENABLE_CC1101=1` for full driver
- **SD library** — `*.remote` / JSON under `/home/default/remotes`
- **Scan** — IR NEC candidates for TV, fan, AC, projector
- **Legacy** — CyberThreatGotchi HTTP client archived in `legacy/ctg_client/`

## Hardware

Full BOM and GPIO tables: [docs/HARDWARE.md](docs/HARDWARE.md)

## Contents

| Path | Role |
|------|------|
| `platformio/` | Multi-transport firmware |
| `remote_codec.py` | Host-side JSON profile validation |
| `docs/CARDPUTER.md` | Flash & keyboard map |
| `legacy/ctg_client/` | Archived CTG status client (v1.x) |

## Quick start

```powershell
cd C:\Users\Owner\Projects\Remote-Possibility\platformio
py -m platformio run -e m5stack-cardputer
```

Package `firmware.bin` → `remote_possibility.bin` for [M5_OS-Cardputer](https://github.com/salvador-Data/M5_OS-Cardputer).

## Firmware download

Prebuilt Cardputer binary (PlatformIO env `m5stack-cardputer`):

| Asset | URL |
|-------|-----|
| `firmware.bin` | [Latest GitHub release](https://github.com/salvador-Data/Remote-Possibility/releases/latest/download/firmware.bin) |

Install via **M5 OS** manifest OTA or copy to SD as `remote_possibility.bin` under `/apps/remote_possibility/` (see [docs/CARDPUTER.md](docs/CARDPUTER.md)). Verify **SHA-256** in [M5_OS manifest.example.json](https://github.com/salvador-Data/M5_OS-Cardputer/blob/main/data/manifest.example.json) before flash.

Tagged releases are built by [`.github/workflows/release.yml`](.github/workflows/release.yml) on `v*` tags.

## Authorized use

Only on **devices and spaces you own** or where you have **written permission**. IR/RF/sub-GHz replay can control third-party equipment — obtain consent in shared buildings.

*Partner fulfillment for module kits; Philadelphia direct ship for assembled Cardputer SKUs.*
