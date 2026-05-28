#pragma once
#include "ir_service.h"
#include "storage.h"
enum class Transport : uint8_t { Ir, Rf433, Rf315, SubGhz, Count };
const char* transportSlug(Transport t);
const char* transportLabel(Transport t);
Transport transportFromSlug(const char* slug);
void transportInitAll();
bool transportReady(Transport t);
bool transportSend(const RemoteCommand& cmd, Transport t);
bool transportLearn(Transport t, uint32_t timeoutMs, RemoteCommand& out);
