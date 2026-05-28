#include "rf_service.h"

#include <RCSwitch.h>

#include "config.h"

static RCSwitch gTx;
static RCSwitch gRx;
static bool gReady = false;

void rfInit() {
    gTx.enableTransmit(RF_TX_PIN);
    gTx.setProtocol(1);
    gRx.enableReceive(RF_RX_PIN);
    gReady = true;
}

bool rfReady() { return gReady; }

bool rfSend(uint32_t code, uint8_t bits, uint8_t protocol) {
    if (!gReady) rfInit();
    gTx.setProtocol(protocol);
    gTx.send(code, bits);
    return true;
}

bool rfCapture(RfCapture& out, uint32_t timeoutMs) {
    if (!gReady) rfInit();
    out = RfCapture{};
    const unsigned long start = millis();
    while (millis() - start < timeoutMs) {
        if (gRx.available()) {
            out.valid = true;
            out.code = gRx.getReceivedValue();
            out.bits = static_cast<uint8_t>(gRx.getReceivedBitlength());
            out.protocol = static_cast<uint8_t>(gRx.getReceivedProtocol());
            gRx.resetAvailable();
            return out.code != 0;
        }
        delay(20);
    }
    return false;
}
