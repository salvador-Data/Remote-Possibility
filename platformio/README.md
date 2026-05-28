# PlatformIO — Remote Possibility

Universal **IR remote** firmware for **M5Stack Cardputer** (ST7789 240×135, QWERTY). Send, learn, library browse, and lab scan mode.

Host helpers: [../remote_codec.py](../remote_codec.py) · Full guide: [../docs/CARDPUTER.md](../docs/CARDPUTER.md) · Wiring: [../docs/HARDWARE.md](../docs/HARDWARE.md)

## Prerequisites

- [PlatformIO](https://platformio.org/)
- microSD with M5 OS VFS paths (optional but recommended for library)
- External IR receiver on Grove G1 for learn mode

## Build & flash

```powershell
cd C:\Users\Owner\Projects\Remote-Possibility\platformio
pio run -e m5stack-cardputer
pio run -e m5stack-cardputer -t upload
pio device monitor
```

GPIO defaults in `platformio.ini`: `IR_TX_PIN=44`, `IR_RX_PIN=1`.

## M5 OS package

```text
.pio/build/m5stack-cardputer/firmware.bin  →  remote_possibility.bin
```

Install via [M5_OS-Cardputer](https://github.com/salvador-Data/M5_OS-Cardputer) SD paths `/apps/remote_possibility/`.

## Screen flow

| Screen | Content |
|--------|---------|
| Home | Library · Learn · Scan · About |
| Library | Category filter → remote list → per-command send |
| Learn | Capture NEC/raw → save JSON to SD |
| Scan | Brand profiles → send candidate → y/n save |

*Control only devices you own or have written permission to operate.*
