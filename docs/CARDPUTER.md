# Remote Possibility — M5 Cardputer universal IR remote

**Hacker Planet LLC** · Authorized lab use only — control devices you own or have written permission to operate.

Keyboard-first **infrared remote** for TV, fan, AC, projector, and other IR-controlled gear. Learn OEM remotes, store JSON libraries on SD, and run guided scan mode for common NEC patterns.

## Architecture

```
  OEM remote ──IR──► Grove RX (GPIO1)     M5 Cardputer
                     Learn / decode  ◄──►  IR TX GPIO44 ──► TV / fan / AC
                     JSON library on SD /home/default/remotes/
```

No Wi-Fi required for send/learn. Scan mode is an **authorized lab demo** — confirm each code with **y** before saving.

## Desktop test (host helpers)

```powershell
cd C:\Users\Owner\Projects\Remote-Possibility
python -m pytest tests/ -q
python remote_codec.py
```

Validate JSON before copying to SD:

```powershell
python -c "from remote_codec import load_remote_file; print(load_remote_file('data/remotes/examples/samsung_tv_lab.json'))"
```

## PlatformIO firmware

```powershell
cd C:\Users\Owner\Projects\Remote-Possibility\platformio
pio run -e m5stack-cardputer
pio run -e m5stack-cardputer -t upload
```

Copy `platformio/.pio/build/m5stack-cardputer/firmware.bin` to M5 OS package name `remote_possibility.bin` (SD `/apps/remote_possibility/` per [M5_OS-Cardputer](https://github.com/salvador-Data/M5_OS-Cardputer)).

### SD setup

1. Format microSD; boot **M5 OS** once to create VFS folders.
2. Copy `data/remotes/examples/*.json` → **`/home/default/remotes/`**
3. Flash Remote Possibility or install via manifest.

Hardware wiring: [HARDWARE.md](HARDWARE.md)

### Keyboard map

| Key | Action |
|-----|--------|
| `;` / `w` | Move up |
| `.` / `s` | Move down |
| Enter | Select / send / save |
| `` ` `` | Back |
| `l` | Learn mode (from home) |
| `f` | Scan mode (from home) |
| `x` | Send highlighted command |
| `y` / `n` | Scan hit yes / try next |

### Screens

| Screen | Purpose |
|--------|---------|
| Home | Library, learn, scan, about |
| Library | Category → remote list → command send |
| Learn | Capture IR from OEM remote → save JSON |
| Scan | Category → brand → try NEC candidates with confirm |

## Remote JSON format

```json
{
  "v": 1,
  "name": "Living Room TV",
  "category": "tv",
  "brand": "Samsung",
  "protocol": "NEC",
  "address": 57568,
  "commands": [
    {"label": "power", "protocol": "NEC", "address": 57568, "command": 16575}
  ]
}
```

Optional `"raw": [9000, 4500, ...]` array per command for unknown protocols.

## Troubleshooting

| Symptom | Fix |
|---------|-----|
| No IR received in learn | Wire receiver to Grove **G1**; point OEM remote at module |
| TV ignores sends | Aim Cardputer IR window; try scan mode or re-learn |
| Empty library | Copy JSON to `/home/default/remotes/`; remount SD |
| SD not mounted | Insert card before boot; check M5 OS file browser |

## Legacy CTG client

CyberThreatGotchi Wi-Fi status polling lived in v1.x firmware. It is archived under [`legacy/ctg_client/`](../legacy/ctg_client/). CTG field status remains on the main CTG web UI.

## Ecosystem

- [M5_OS-Cardputer](https://github.com/salvador-Data/M5_OS-Cardputer) — launcher + `remote_possibility.bin`
- [BLE-Bot-Cardputer](https://github.com/salvador-Data/BLE-Bot-Cardputer) — separate BLE scout SKU (same hardware)
- [cyberThreatGotchi](https://github.com/salvador-Data/cyberThreatGotchi) — edge IPS (not required for IR remote)
