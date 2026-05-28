# Remote Possibility — M5 Cardputer CTG client

**Hacker Planet LLC** · Philadelphia, PA · [cardputer.html](https://hackerplanet.dev/cardputer.html)

Authorized-lab **CyberThreatGotchi remote status client** for the M5Stack Cardputer. Polls your CTG instance `/api/status` over Wi‑Fi and shows mood, level, and recent threats on the handheld keyboard UI.

| SKU | Stripe key | Retail |
|-----|------------|--------|
| Remote Possibility | `remotePossibility` | **$89.99** (+ tax/shipping) |

Philadelphia-assembled units ship pre-flashed. DIY builders flash from this repo.

## Contents

- `ctg_status.py` — MicroPython client
- `platformio/` — Arduino / PlatformIO firmware (`pio run -t upload`)

Configure `CTG_HOST`, Wi‑Fi, and polling interval per [docs/CARDPUTER.md](docs/CARDPUTER.md).

## M5 OS launcher

Install **M5 OS** first so packages can load from SD or manifest: [M5_OS-Cardputer](https://github.com/salvador-Data/M5_OS-Cardputer).

## Shop & docs

- Product page: https://hackerplanet.dev/cardputer.html#remote-possibility
- Pricing authority: [PRODUCT_PRICING.md](https://github.com/salvador-Data/cyberThreatGotchi/blob/main/docs/PRODUCT_PRICING.md)
- Main CTG repo: [cyberThreatGotchi](https://github.com/salvador-Data/cyberThreatGotchi)

*Authorized networks and systems you own or have written permission to test only.*
