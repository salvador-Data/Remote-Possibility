# Remote Possibility — M5 Cardputer CTG client

**Hacker Planet LLC** · Philadelphia, PA · [cardputer.html](https://hackerplanet.dev/cardputer.html#remote-possibility)

Authorized-lab **CyberThreatGotchi remote status client** for the M5Stack Cardputer. Polls your CTG instance `GET /api/status` over Wi‑Fi and shows mood, level, and recent threats on the ST7789 240×135 display with full QWERTY configuration.

| SKU | Stripe key | Retail |
|-----|------------|--------|
| Remote Possibility | `remotePossibility` | **$89.99** (+ tax/shipping) |

Philadelphia-assembled units ship pre-flashed. DIY builders build from this repo.

## Features

- Wi-Fi via **NVS** saved credentials, **serial `SET`**, or optional SD `/config/remote_possibility.cfg` (M5 OS paths)
- HTTP poll with **timeout**, **reconnect**, and manual **`r`** refresh
- **Settings UI** on keyboard: CTG host, port, SSID, password, poll interval
- PlatformIO env **`m5stack-cardputer`** (`M5Cardputer` + `ArduinoJson`)
- MicroPython **`ctg_status.py`** with desktop-testable parsers

## Contents

| Path | Role |
|------|------|
| `platformio/` | Arduino firmware — `pio run -e m5stack-cardputer` |
| `ctg_status.py` | MicroPython client |
| `docs/CARDPUTER.md` | Flash, keyboard map, troubleshooting |
| `tests/test_ctg_status.py` | Parser unit tests |

## Quick start

```powershell
cd C:\Users\Owner\Projects\Remote-Possibility\platformio
pio run -e m5stack-cardputer
```

USB serial **115200**: `HELP`, `SET HOST 192.168.1.50`, `SET SSID MyLab`, `SET PASS secret`, `WIFI`.

## M5 OS launcher

Install **M5 OS** first: [M5_OS-Cardputer](https://github.com/salvador-Data/M5_OS-Cardputer). Package firmware as `remote_possibility.bin`.

## Shop & docs

- Product: https://hackerplanet.dev/cardputer.html#remote-possibility
- CTG API: [cyberThreatGotchi docs/WEB.md](https://github.com/salvador-Data/cyberThreatGotchi/blob/main/docs/WEB.md)
- Pricing: [PRODUCT_PRICING.md](https://github.com/salvador-Data/cyberThreatGotchi/blob/main/docs/PRODUCT_PRICING.md)

*Authorized networks and systems you own or have written permission to test only.*
