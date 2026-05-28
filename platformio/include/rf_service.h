#pragma once
#include <stdint.h>
struct RfCapture { bool valid=false; uint32_t code=0; uint8_t bits=24; uint8_t protocol=1; };
void rfInit(); bool rfReady(); bool rfSend(uint32_t code, uint8_t bits, uint8_t protocol);
bool rfCapture(RfCapture& out, uint32_t timeoutMs);
