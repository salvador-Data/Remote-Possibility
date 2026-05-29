"""Release URL and README download docs stay aligned with M5 OS manifest."""

from __future__ import annotations

from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
DOWNLOAD_URL = (
    "https://github.com/salvador-Data/Remote-Possibility/releases/latest/download/firmware.bin"
)


def test_readme_documents_firmware_download():
    text = (ROOT / "README.md").read_text(encoding="utf-8")
    assert "firmware.bin" in text
    assert DOWNLOAD_URL in text
    assert "M5_OS-Cardputer" in text


def test_cardputer_doc_documents_firmware_download():
    text = (ROOT / "docs" / "CARDPUTER.md").read_text(encoding="utf-8")
    assert DOWNLOAD_URL in text
    assert "remote_possibility.bin" in text


def test_release_workflow_publishes_firmware_bin():
    text = (ROOT / ".github" / "workflows" / "release.yml").read_text(encoding="utf-8")
    assert "firmware.bin" in text
    assert "m5stack-cardputer" in text
