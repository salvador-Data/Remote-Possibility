"""
Remote Possibility — IR remote JSON codec (host-side helpers)

Parse and validate remote profile files stored on SD at /home/default/remotes/.
"""

from __future__ import annotations

import json
import re
from pathlib import Path
from typing import Any

VALID_CATEGORIES = frozenset({"tv", "fan", "ac", "projector", "audio", "light", "other"})
VALID_TRANSPORTS = frozenset({"ir", "rf433", "rf315", "subghz", "cc1101"})
HEX_RE = re.compile(r"^(0x)?[0-9a-fA-F]+$")


def parse_hex(value: Any) -> int:
    """Accept int or hex string (0x prefix optional)."""
    if isinstance(value, int):
        return value
    text = str(value).strip()
    if not HEX_RE.match(text):
        raise ValueError("invalid hex: {!r}".format(value))
    if text.lower().startswith("0x"):
        return int(text, 16)
    if any(c in text.lower() for c in "abcdef"):
        return int(text, 16)
    return int(text)


def parse_command(obj: dict[str, Any]) -> dict[str, Any]:
    label = str(obj.get("label", "cmd"))
    protocol = str(obj.get("protocol", "NEC")).upper()
    address = parse_hex(obj.get("address", 0))
    command = parse_hex(obj.get("command", 0))
    raw = obj.get("raw")
    raw_list: list[int] = []
    if raw is not None:
        if not isinstance(raw, list):
            raise ValueError("raw must be a list")
        raw_list = [int(x) for x in raw]
    return {
        "label": label,
        "protocol": protocol,
        "address": address,
        "command": command,
        "raw": raw_list,
    }


def parse_remote_profile(data: dict[str, Any]) -> dict[str, Any]:
    """Normalize a remote profile JSON document."""
    category = str(data.get("category", "other")).lower()
    if category not in VALID_CATEGORIES:
        raise ValueError("invalid category: {}".format(category))
    transport = str(data.get("transport", "ir")).lower()
    if transport not in VALID_TRANSPORTS:
        raise ValueError("invalid transport: {}".format(transport))
    commands_in = data.get("commands") or []
    if not isinstance(commands_in, list):
        raise ValueError("commands must be a list")
    commands = [parse_command(c) for c in commands_in if isinstance(c, dict)]
    return {
        "v": int(data.get("v", 1)),
        "name": str(data.get("name", "Remote")),
        "category": category,
        "transport": transport,
        "brand": str(data.get("brand", "")),
        "protocol": str(data.get("protocol", "NEC")).upper(),
        "address": parse_hex(data.get("address", 0)),
        "commands": commands,
    }


def load_remote_file(path: str | Path) -> dict[str, Any]:
    text = Path(path).read_text(encoding="utf-8")
    return parse_remote_profile(json.loads(text))


def slugify(text: str) -> str:
    out = []
    for ch in text.lower():
        if ch.isalnum():
            out.append(ch)
        elif ch in (" ", "-", "_"):
            out.append("_")
    slug = "".join(out).strip("_")
    return slug or "remote"


def scan_candidate_to_profile(
    *,
    category: str,
    brand: str,
    label: str,
    protocol: str,
    address: int,
    command: int,
) -> dict[str, Any]:
    """Build a minimal profile from a lab scan hit."""
    return {
        "v": 1,
        "name": "{} {}".format(brand, category),
        "category": category,
        "brand": brand,
        "protocol": protocol.upper(),
        "address": address,
        "commands": [
            {
                "label": label,
                "protocol": protocol.upper(),
                "address": address,
                "command": command,
            }
        ],
    }
