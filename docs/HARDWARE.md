# Remote Possibility — hardware BOM & wiring

**Hacker Planet LLC** · M5Stack Cardputer (ESP32-S3) · multi-transport universal remote

## Kit BOM (DIY / partner fulfillment)

| Module | Typical part | Est. eBay | Role |
|--------|----------------|-----------|------|
| M5Stack Cardputer or ADV | ESP32-S3 + keyboard + ST7789 | ~$50 | Host |
| 38 kHz IR transceiver | KY-019 / VS1838B breakout | ~$3–6 | TV/AC IR learn + send |
| 433 MHz RF pair | STX882 + RXB6 (or superheterodyne kit) | ~$4–8 | Garage/fan RF OOK |
| 315 MHz RF pair (optional) | US-market gate remotes | ~$4–8 | Alternate OOK band |
| CC1101 sub-GHz | CC1101 433 MHz SPI module | ~$5–10 | Sub-GHz learn/replay (300–928 MHz) |
| microSD | 8–32 GB | ~$5 | Profile library |
| Jumper wires | HY2.0-4P + dupont | ~$3 | Field wiring |

Philadelphia SKU **Remote Possibility** ($99.99) ships Cardputer + SD + quick-start; IR module recommended in-box, RF/CC1101 as documented add-ons.

## Transport summary

| Transport | `transport=` slug | Default GPIO | Library |
|-----------|-------------------|--------------|---------|
| Infrared 38 kHz | `ir` | TX **44**, RX **1** | Arduino-IRremote |
| RF 433 MHz OOK | `rf433` | TX **43**, RX **2** | RCSwitch |
| RF 315 MHz OOK | `rf315` | Same pins — **swap module** to 315 MHz hardware | RCSwitch |
| Sub-GHz CC1101 | `subghz` / `cc1101` | SPI见下表 | Stub until `RP_ENABLE_CC1101=1` |

Profiles on SD use `transport=` plus per-command `protocol`, `address`, `command`, optional `raw=`.

## IR module (required for TV/fan/AC scan)

| Module pin | Cardputer GPIO | Function |
|------------|----------------|----------|
| VCC | **3.3 V** | Never 5 V on GPIO bank |
| GND | **GND** | Common ground |
| TX / LED | **GPIO 44** | IR transmit |
| RX / OUT | **GPIO 1** | IR receive |

```
  HY2.0-4P          IR module
  3.3V  ─────────  VCC
  GND   ─────────  GND
  G44   ─────────  TX
  G1    ─────────  RX
```

## RF 433 / 315 MHz OOK

Use separate **433 MHz** or **315 MHz** transmitter/receiver boards — firmware uses the same GPIO; only one band module should be active at a time.

| Module pin | Cardputer GPIO | Function |
|------------|----------------|----------|
| VCC | **3.3 V** (check module — some need 5 V on VCC only) | Power per datasheet |
| GND | **GND** | |
| DATA (TX) | **GPIO 43** | OOK transmit |
| DATA (RX) | **GPIO 2** | OOK receive |

**Authorized use:** RF replay affects neighbors’ gear — test only on property you control.

## CC1101 sub-GHz (SPI)

Dedicated SPI bus — **do not** share CS with microSD (SD uses 12/14/39/40).

| CC1101 | Cardputer GPIO |
|--------|----------------|
| VCC | 3.3 V |
| GND | GND |
| CS | **8** |
| SCK | **36** |
| MISO | **37** |
| MOSI | **35** |
| GDO0 | **9** |

Enable full driver in `platformio.ini`:

```ini
build_flags = ... -DRP_ENABLE_CC1101=1
lib_deps = ... LSatan/SmartRC-CC1101-Driver-Lib@^2.5.0
```

Until enabled, UI learn/send for **Sub-GHz** shows wiring hint; IR and RF work with modules attached.

## microSD (M5 OS)

| Signal | GPIO |
|--------|------|
| CS | 12 |
| SCLK | 14 |
| MISO | 39 |
| MOSI | 40 |

Paths: `/home/default/remotes/*.remote` or `/remotes/` — JSON examples in `data/remotes/examples/`.

## Pin overrides

```ini
build_flags =
    -DIR_TX_PIN=44
    -DIR_RX_PIN=1
    -DRF_TX_PIN=43
    -DRF_RX_PIN=2
    -DCC1101_CS=8
```

*Home, lab, and equipment you own or may control only.*
