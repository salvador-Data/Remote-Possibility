#include "ir_service.h"

#include <IRremote.hpp>
#include <M5Cardputer.h>

#include "config.h"

static bool gIrReady = false;

void irInit() {
    IrSender.begin(IR_TX_PIN);
    IrReceiver.begin(IR_RX_PIN, ENABLE_LED_FEEDBACK);
    gIrReady = true;
}

bool irSendNec(uint16_t address, uint16_t command, uint8_t repeats) {
    if (!gIrReady) irInit();
    IrSender.sendNEC(address, command, repeats);
    return true;
}

bool irSendRaw(const uint16_t* raw, uint16_t len, uint8_t freqKhz) {
    if (!gIrReady) irInit();
    if (!raw || len == 0) return false;
    IrSender.sendRaw(raw, len, freqKhz);
    return true;
}

const char* irProtocolName(uint8_t protocol) {
    switch (protocol) {
        case NEC:
            return "NEC";
        case SAMSUNG:
            return "SAMSUNG";
        case LG:
            return "LG";
        case SONY:
            return "SONY";
        case RC5:
            return "RC5";
        case RC6:
            return "RC6";
        case PANASONIC:
            return "PANASONIC";
        case JVC:
            return "JVC";
        case DENON:
            return "DENON";
        default:
            return "UNKNOWN";
    }
}

static void fillCaptureFromDecode(IrCapture& out) {
    const auto& d = IrReceiver.decodedIRData;
    out.valid = true;
    strncpy(out.protocol, irProtocolName(d.protocol), sizeof(out.protocol) - 1);
    out.address = static_cast<uint16_t>(d.address);
    out.command = static_cast<uint16_t>(d.command);
    out.bits = d.numberOfBits;
    out.rawLen = 0;
}

bool irCapture(IrCapture& out, uint32_t timeoutMs) {
    if (!gIrReady) irInit();
    out = IrCapture{};
    const unsigned long start = millis();
    while (millis() - start < timeoutMs) {
        if (IrReceiver.decode()) {
            fillCaptureFromDecode(out);
            IrReceiver.resume();
            return out.valid;
        }
        delay(20);
        M5Cardputer.update();
    }
    return false;
}
