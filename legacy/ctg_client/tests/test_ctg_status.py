"""Tests for ctg_status.py helpers (no hardware required)."""

from __future__ import annotations

import importlib.util
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
MODULE = ROOT / "ctg_status.py"


def _load():
    spec = importlib.util.spec_from_file_location("ctg_status", MODULE)
    mod = importlib.util.module_from_spec(spec)
    assert spec and spec.loader
    sys.modules["ctg_status"] = mod
    spec.loader.exec_module(mod)
    return mod


def test_build_status_url():
    mod = _load()
    assert mod.build_status_url("192.168.1.50", 8765) == "http://192.168.1.50:8765/api/status"


def test_parse_status_payload_minimal():
    mod = _load()
    row = mod.parse_status_payload({"gotchi": {"mood": "alert", "level": 3}, "threats": []})
    assert row["mood"] == "alert"
    assert row["mood_label"] == "ALERT"
    assert row["level"] == 3
    assert row["last_ip"] == ""


def test_parse_status_payload_with_threat():
    mod = _load()
    payload = {
        "gotchi": {
            "name": "Cipherhorn",
            "mood": "attack",
            "level": 5,
            "threats_blocked": 2,
            "threats_seen": 4,
            "status_line": "Blocking intruder",
        },
        "threats": [{"source_ip": "10.0.0.5", "severity": "high", "action_taken": "drop"}],
    }
    row = mod.parse_status_payload(payload)
    assert row["name"] == "Cipherhorn"
    assert row["blocked"] == 2
    assert row["seen"] == 4
    assert row["last_ip"] == "10.0.0.5"
    assert row["last_sev"] == "high"
    assert row["last_action"] == "drop"
