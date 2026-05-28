#include "cc1101_service.h"

#include "config.h"

void cc1101Init() {}

bool cc1101Ready() {
#if defined(RP_ENABLE_CC1101) && RP_ENABLE_CC1101
    return false;
#else
    return false;
#endif
}

bool cc1101Send(uint32_t value, uint16_t bitLength) {
#if defined(RP_ENABLE_CC1101) && RP_ENABLE_CC1101
    (void)value;
    (void)bitLength;
    return false;
#else
    (void)value;
    (void)bitLength;
    return false;
#endif
}

bool cc1101Capture(SubGhzCapture& out, uint32_t timeoutMs) {
    (void)timeoutMs;
    out = SubGhzCapture{};
#if defined(RP_ENABLE_CC1101) && RP_ENABLE_CC1101
    return false;
#else
    return false;
#endif
}
