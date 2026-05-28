#pragma once
#include <stdint.h>
struct SubGhzCapture { bool valid=false; uint32_t value=0; uint16_t bitLength=0; };
void cc1101Init(); bool cc1101Ready(); bool cc1101Send(uint32_t value, uint16_t bitLength);
bool cc1101Capture(SubGhzCapture& out, uint32_t timeoutMs);
