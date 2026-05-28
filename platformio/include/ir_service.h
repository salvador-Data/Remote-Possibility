#pragma once

#include <stdint.h>

#include "config.h"

struct IrCapture {
    bool valid = false;
    char protocol[16] = {};
    uint16_t address = 0;
    uint16_t command = 0;
    uint16_t raw[kMaxRawLen] = {};
    uint16_t rawLen = 0;
    uint8_t bits = 0;
};

void irInit();
bool irSendNec(uint16_t address, uint16_t command, uint8_t repeats = 2);
bool irSendRaw(const uint16_t* raw, uint16_t len, uint8_t freqKhz = 38);
bool irCapture(IrCapture& out, uint32_t timeoutMs = 15000);
const char* irProtocolName(uint8_t protocol);
