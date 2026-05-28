"""Tests for remote_codec.py (no hardware required)."""

from __future__ import annotations

import json

import pytest

from remote_codec import (
    load_remote_file,
    parse_command,
    parse_hex,
    parse_remote_profile,
    scan_candidate_to_profile,
    slugify,
)


def test_parse_hex_int_and_string():
    assert parse_hex(0xE0E0) == 57568
    assert parse_hex("0x40BF") == 16575
    assert parse_hex("40BF") == 16575


def test_parse_command_minimal():
    row = parse_command({"label": "power", "address": "0xE0E0", "command": "0x40BF"})
    assert row["label"] == "power"
    assert row["address"] == 0xE0E0
    assert row["command"] == 0x40BF
    assert row["raw"] == []


def test_parse_remote_profile_transport():
    payload = {
        "name": "Gate",
        "category": "other",
        "transport": "rf433",
        "commands": [{"label": "open", "address": 1, "command": 2}],
    }
    row = parse_remote_profile(payload)
    assert row["transport"] == "rf433"


def test_parse_remote_profile():
    payload = {
        "name": "Living Room TV",
        "category": "tv",
        "brand": "Samsung",
        "protocol": "NEC",
        "address": 0xE0E0,
        "commands": [{"label": "power", "address": 0xE0E0, "command": 0x40BF}],
    }
    row = parse_remote_profile(payload)
    assert row["category"] == "tv"
    assert len(row["commands"]) == 1
    assert row["commands"][0]["command"] == 0x40BF


def test_parse_remote_profile_invalid_category():
    with pytest.raises(ValueError):
        parse_remote_profile({"category": "spaceship", "commands": []})


def test_scan_candidate_to_profile():
    prof = scan_candidate_to_profile(
        category="tv",
        brand="Samsung",
        label="power",
        protocol="NEC",
        address=0xE0E0,
        command=0x40BF,
    )
    assert prof["commands"][0]["command"] == 0x40BF


def test_slugify():
    assert slugify("Living Room TV") == "living_room_tv"


def test_load_example_json(tmp_path):
    data = {
        "v": 1,
        "name": "Demo Fan",
        "category": "fan",
        "brand": "Generic",
        "protocol": "NEC",
        "address": 0,
        "commands": [{"label": "power", "address": "0x00FF", "command": "0x42BD"}],
    }
    path = tmp_path / "demo_fan.json"
    path.write_text(json.dumps(data), encoding="utf-8")
    row = load_remote_file(path)
    assert row["name"] == "Demo Fan"
