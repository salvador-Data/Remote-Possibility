"""
CyberThreatGotchi remote status — M5 Cardputer (MicroPython)

Authorized lab networks only. Polls GET /api/status on your CTG edge node.
Desktop test: python -m pytest tests/ -q
"""

from __future__ import annotations

import json
import time
from typing import Any
from urllib.error import URLError
from urllib.request import Request, urlopen

CTG_HOST = "192.168.1.50"
CTG_PORT = 8765
POLL_SEC = 4
HTTP_TIMEOUT = 5.0

MOOD_LABEL = {
    "idle": "IDLE",
    "happy": "OK",
    "alert": "ALERT",
    "attack": "BLOCK",
    "sleep": "ZZZ",
    "feed": "FOOD",
    "defend": "DEF",
}


def build_status_url(host: str, port: int) -> str:
    return "http://{}:{}/api/status".format(host, int(port))


def parse_status_payload(data: dict[str, Any]) -> dict[str, Any]:
    """Normalize /api/status JSON for display helpers."""
    g = data.get("gotchi") or {}
    threats = data.get("threats") or []
    last = threats[0] if threats else {}
    return {
        "name": g.get("name", "Cipherhorn"),
        "mood": g.get("mood", "idle"),
        "mood_label": MOOD_LABEL.get(g.get("mood", "idle"), str(g.get("mood", "idle")).upper()),
        "level": int(g.get("level", 1)),
        "blocked": int(g.get("threats_blocked", 0)),
        "seen": int(g.get("threats_seen", 0)),
        "status_line": str(g.get("status_line", ""))[:32],
        "last_ip": str(last.get("source_ip", "")) if last else "",
        "last_sev": str(last.get("severity", "")) if last else "",
        "last_action": str(last.get("action_taken", "")) if last else "",
    }


def fetch_status(host: str = CTG_HOST, port: int = CTG_PORT, timeout: float = HTTP_TIMEOUT) -> dict[str, Any]:
    url = build_status_url(host, port)
    req = Request(url, headers={"User-Agent": "Remote-Possibility/1.0"})
    with urlopen(req, timeout=timeout) as resp:
        return json.loads(resp.read().decode("utf-8"))


def draw_screen(data: dict[str, Any], lcd: Any) -> None:
    row = parse_status_payload(data)
    mood = row["mood"]
    lcd.fill(lcd.BLACK)
    lcd.setTextColor(lcd.GREEN)
    lcd.setCursor(4, 4)
    lcd.print("CTG " + row["name"])
    lcd.setCursor(4, 20)
    lcd.print(row["mood_label"] + " Lv" + str(row["level"]))
    lcd.setCursor(4, 36)
    lcd.print("Blk:" + str(row["blocked"]))
    lcd.print(" See:" + str(row["seen"]))
    lcd.setCursor(4, 52)
    lcd.print(row["status_line"])
    lcd.setTextColor(lcd.YELLOW if mood in ("alert", "attack") else lcd.WHITE)
    lcd.setCursor(4, 72)
    if row["last_ip"]:
        lcd.print(row["last_ip"][:16])
        lcd.setCursor(4, 88)
        lcd.print((row["last_sev"] + " " + row["last_action"])[:20])
    else:
        lcd.print("No threats yet")


def main() -> None:
    try:
        from M5 import LCD  # type: ignore

        lcd = LCD
    except ImportError:
        print("MicroPython LCD not found. Desktop test:")
        print("  python ctg_status.py  # with CTG --web running")
        try:
            data = fetch_status()
            print(json.dumps(parse_status_payload(data), indent=2))
        except URLError as exc:
            print("CTG unreachable:", exc)
        return

    while True:
        try:
            data = fetch_status()
            draw_screen(data, lcd)
        except Exception:
            lcd.fill(lcd.BLACK)
            lcd.setCursor(4, 40)
            lcd.print("CTG offline")
        time.sleep(POLL_SEC)


if __name__ == "__main__":
    main()
