#include "remote_transport.h"

#include <string.h>

#include "cc1101_service.h"
#include "rf_service.h"

const char* transportSlug(Transport t) {
    switch (t) {
        case Transport::Ir:
            return "ir";
        case Transport::Rf433:
            return "rf433";
        case Transport::Rf315:
            return "rf315";
        case Transport::SubGhz:
            return "subghz";
        default:
            return "ir";
    }
}

const char* transportLabel(Transport t) {
    switch (t) {
        case Transport::Ir:
            return "IR (38 kHz)";
        case Transport::Rf433:
            return "RF 433 MHz";
        case Transport::Rf315:
            return "RF 315 MHz";
        case Transport::SubGhz:
            return "CC1101 sub-GHz";
        default:
            return "Unknown";
    }
}

Transport transportFromSlug(const char* slug) {
    if (!slug) return Transport::Ir;
    if (strcasecmp(slug, "rf433") == 0) return Transport::Rf433;
    if (strcasecmp(slug, "rf315") == 0) return Transport::Rf315;
    if (strcasecmp(slug, "subghz") == 0 || strcasecmp(slug, "cc1101") == 0) {
        return Transport::SubGhz;
    }
    return Transport::Ir;
}

void transportInitAll() {
    irInit();
    rfInit();
    cc1101Init();
}

bool transportReady(Transport t) {
    switch (t) {
        case Transport::Ir:
            return true;
        case Transport::Rf433:
        case Transport::Rf315:
            return rfReady();
        case Transport::SubGhz:
            return cc1101Ready();
        default:
            return false;
    }
}

static uint32_t rfCodeFromCommand(const RemoteCommand& cmd) {
    if (cmd.address && cmd.command) {
        return (static_cast<uint32_t>(cmd.address) << 16) | cmd.command;
    }
    if (cmd.command) return cmd.command;
    return cmd.address;
}

bool transportSend(const RemoteCommand& cmd, Transport t) {
    switch (t) {
        case Transport::Ir:
            if (cmd.hasRaw && cmd.rawLen > 0) {
                return irSendRaw(cmd.raw, cmd.rawLen);
            }
            return irSendNec(cmd.address, cmd.command);
        case Transport::Rf433:
            return rfSend(rfCodeFromCommand(cmd), 24, 1);
        case Transport::Rf315:
            return rfSend(rfCodeFromCommand(cmd), 24, 1);
        case Transport::SubGhz:
            return cc1101Send(rfCodeFromCommand(cmd), 24);
        default:
            return false;
    }
}

static void fillLearnedFromIr(const IrCapture& cap, RemoteCommand& out) {
    out.label = "learned";
    out.protocol = cap.protocol;
    out.address = cap.address;
    out.command = cap.command;
    if (cap.rawLen > 0) {
        out.hasRaw = true;
        out.rawLen = cap.rawLen;
        memcpy(out.raw, cap.raw, cap.rawLen * sizeof(uint16_t));
    }
}

static void fillLearnedFromRf(const RfCapture& cap, RemoteCommand& out) {
    out.label = "learned";
    out.protocol = "OOK";
    out.address = static_cast<uint16_t>((cap.code >> 16) & 0xFFFF);
    out.command = static_cast<uint16_t>(cap.code & 0xFFFF);
}

static void fillLearnedFromSubGhz(const SubGhzCapture& cap, RemoteCommand& out) {
    out.label = "learned";
    out.protocol = "CC1101";
    out.address = static_cast<uint16_t>((cap.value >> 16) & 0xFFFF);
    out.command = static_cast<uint16_t>(cap.value & 0xFFFF);
}

bool transportLearn(Transport t, uint32_t timeoutMs, RemoteCommand& out) {
    out = RemoteCommand{};
    switch (t) {
        case Transport::Ir: {
            IrCapture cap;
            if (!irCapture(cap, timeoutMs)) return false;
            fillLearnedFromIr(cap, out);
            return true;
        }
        case Transport::Rf433:
        case Transport::Rf315: {
            RfCapture cap;
            if (!rfCapture(cap, timeoutMs)) return false;
            fillLearnedFromRf(cap, out);
            return true;
        }
        case Transport::SubGhz: {
            SubGhzCapture cap;
            if (!cc1101Capture(cap, timeoutMs)) return false;
            fillLearnedFromSubGhz(cap, out);
            return true;
        }
        default:
            return false;
    }
}
