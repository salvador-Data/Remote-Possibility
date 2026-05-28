# Remote Possibility — Cardputer + CyberThreatGotchi

**Hacker Planet LLC** · Authorized lab use only — networks and systems you own or have written permission to test.

Show **Cipherhorn mood** and the latest threat on your M5Stack Cardputer by polling the CTG web API over Wi-Fi.

## Architecture

```
  BPI-R3 Mini (CTG)                    M5Stack Cardputer
  ┌─────────────────┐                  ┌──────────────────────┐
  │ main.py --web   │  Wi-Fi LAN       │ Remote Possibility   │
  │ :8765 /api/status ────────────────►│ GET every 4s (cfg)   │
  └─────────────────┘                  └──────────────────────┘
```

Read-only client — no CTG firmware changes required.

## Desktop test (before flashing)

Terminal A — start CTG:

```powershell
cd C:\Users\Owner\Projects\cyberThreatGotchi
python main.py --simulation --web
```

Terminal B — MicroPython helpers / pytest:

```powershell
cd C:\Users\Owner\Projects\Remote-Possibility
python -m pytest tests/ -q
python ctg_status.py
```

Or from the main CTG repo: `python scripts/cardputer_status.py --host 127.0.0.1 --watch`

## PlatformIO firmware (recommended)

```powershell
cd C:\Users\Owner\Projects\Remote-Possibility\platformio
pio run -e m5stack-cardputer
pio run -e m5stack-cardputer -t upload
```

Copy `platformio/.pio/build/m5stack-cardputer/firmware.bin` to M5 OS package name `remote_possibility.bin` (SD `/firmware/` or `/apps/remote_possibility/` per [M5_OS-Cardputer](https://github.com/salvador-Data/M5_OS-Cardputer)).

### First-time Wi-Fi + CTG URL

1. Flash firmware; open USB serial at **115200**.
2. Type `HELP` for commands, e.g. `SET SSID MyLab`, `SET PASS secret`, `SET HOST 192.168.1.50`, `SET PORT 8765`, `WIFI`.
3. On device: **e** = settings menu, edit host/port/Wi-Fi, **Enter** to save (stored in NVS).

### Keyboard map (status view)

| Key | Action |
|-----|--------|
| `r` | Poll `/api/status` now |
| `e` | Open settings (host, port, Wi-Fi, poll interval) |
| `;` / `w` | Move up (settings menu) |
| `.` / `s` | Move down (settings menu) |
| Enter | Edit selected setting |
| `` ` `` | Back from settings / cancel edit |

### Optional SD config (M5 OS layout)

If microSD is mounted, load ` /config/remote_possibility.cfg` (key=value):

```ini
host=192.168.1.50
port=8765
ssid=YourLabSSID
pass=YourLabPassword
poll_ms=4000
```

NVS values from the keyboard UI override after save; SD file applies on boot before NVS merge.

## MicroPython

Copy `ctg_status.py` to the Cardputer SD payload or run from M5 OS. Edit `CTG_HOST` / `CTG_PORT` at top of file, or use desktop `fetch_status(host, port)`.

## JSON fields used

| Path | Display |
|------|---------|
| `gotchi.mood` | IDLE, ALERT, BLOCK, … |
| `gotchi.name` | Title |
| `gotchi.level` | Level |
| `gotchi.threats_blocked` / `threats_seen` | Counters |
| `gotchi.status_line` | Caption |
| `threats[0].source_ip` | Last attacker |
| `threats[0].severity` | Severity |
| `threats[0].action_taken` | IPS action |

## Troubleshooting

| Symptom | Fix |
|---------|-----|
| `CTG unreachable` | Ping CTG IP; confirm `python main.py --web`; firewall allows **8765** |
| `no WiFi` | Set SSID/pass in settings or serial `SET SSID` / `SET PASS` then `WIFI` |
| Empty threats | Normal until simulation/live traffic |
| Wrong mood | Compare browser `http://<ctg-ip>:8765/api/status` |

## Ecosystem

- [cyberThreatGotchi](https://github.com/salvador-Data/cyberThreatGotchi) — edge IPS + `/api/status`
- [M5_OS-Cardputer](https://github.com/salvador-Data/M5_OS-Cardputer) — launcher + `remote_possibility.bin`
- [BLE-Bot-Cardputer](https://github.com/salvador-Data/BLE-Bot-Cardputer) — separate BLE scout SKU (same hardware)
