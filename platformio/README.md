# PlatformIO — Remote Possibility

Native C++ firmware for **M5Stack Cardputer** (ST7789 240×135, QWERTY). Polls CyberThreatGotchi `GET /api/status` over authorized lab Wi-Fi.

MicroPython: [../ctg_status.py](../ctg_status.py) · Full guide: [../docs/CARDPUTER.md](../docs/CARDPUTER.md)

## Prerequisites

- [PlatformIO](https://platformio.org/)
- CTG web API on the LAN: `python main.py --web` (default port **8765**)

## Build & flash

```powershell
cd C:\Users\Owner\Projects\Remote-Possibility\platformio
pio run -e m5stack-cardputer
pio run -e m5stack-cardputer -t upload
pio device monitor
```

Default compile-time fallbacks (`platformio.ini` `build_flags`): `CTG_HOST`, `CTG_PORT`, `POLL_MS`. Runtime **NVS** + serial `SET` commands override after first save.

## M5 OS package

```text
.pio/build/m5stack-cardputer/firmware.bin  →  remote_possibility.bin
```

Install via [M5_OS-Cardputer](https://github.com/salvador-Data/M5_OS-Cardputer) SD paths `/firmware/` or `/apps/remote_possibility/`.

## Screen layout

| Area | Content |
|------|---------|
| Header | Product name, Wi-Fi line, `host:port` |
| Body | Gotchi name, mood, level, block/seen counts |
| Footer | Status line, last threat IP + action |
| Hint | `r` refresh · `e` settings |

Red/green header reflects last HTTP result; mood row uses color by threat level.

*Authorized networks and systems you own or have written permission to test only.*
