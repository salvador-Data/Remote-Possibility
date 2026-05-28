#pragma once

/** M5 Cardputer multi-transport wiring — see docs/HARDWARE.md */

#ifndef IR_TX_PIN
#define IR_TX_PIN 44
#endif
#ifndef IR_RX_PIN
#define IR_RX_PIN 1
#endif

/** 433 MHz OOK (RXB6 / SRX882 + STX882 class modules) */
#ifndef RF_TX_PIN
#define RF_TX_PIN 43
#endif
#ifndef RF_RX_PIN
#define RF_RX_PIN 2
#endif

/** CC1101 sub-GHz SPI (dedicated bus — do not share SD CS) */
#ifndef CC1101_CS
#define CC1101_CS 8
#endif
#ifndef CC1101_SCK
#define CC1101_SCK 36
#endif
#ifndef CC1101_MISO
#define CC1101_MISO 37
#endif
#ifndef CC1101_MOSI
#define CC1101_MOSI 35
#endif
#ifndef CC1101_GDO0
#define CC1101_GDO0 9
#endif

static const int kSdCs = 12;
static const int kSdSclk = 14;
static const int kSdMiso = 39;
static const int kSdMosi = 40;

static const char* kRemotesPrimary = "/home/default/remotes";
static const char* kRemotesLegacy = "/remotes";

static const char* kPrefsNs = "rp_remote";
static const int kVisibleRows = 6;
static const int kMaxNameLen = 32;
static const int kMaxPathLen = 96;
static const int kMaxCommands = 24;
static const int kMaxRawLen = 200;
static const int kMaxRfBits = 64;
