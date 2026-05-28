# Legacy — CyberThreatGotchi status client

**Archived.** Remote Possibility v1.x was a Wi-Fi HTTP poll client for CTG `GET /api/status`.

Replaced by the universal IR remote firmware in the repo root (`platformio/`). CTG field status is still available from the main [cyberThreatGotchi](https://github.com/salvador-Data/cyberThreatGotchi) web UI and `scripts/cardputer_status.py`.

| Path | Role |
|------|------|
| `platformio/src/main.cpp` | Arduino CTG client |
| `ctg_status.py` | MicroPython CTG client |
| `tests/test_ctg_status.py` | Parser unit tests |

*Authorized networks and systems you own or have written permission to test only.*
