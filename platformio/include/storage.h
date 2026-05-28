#pragma once

#include <Arduino.h>
#include <vector>

#include "config.h"
#include "ir_service.h"

struct RemoteCommand {
    String label;
    String protocol;
    uint16_t address = 0;
    uint16_t command = 0;
    bool hasRaw = false;
    uint16_t raw[kMaxRawLen];
    uint16_t rawLen = 0;
};

struct RemoteProfile {
    String path;
    String name;
    String category;
    String brand;
    String transport = "ir";
    String protocol;
    uint16_t address = 0;
    std::vector<RemoteCommand> commands;
};

bool storageInit();
bool storageReady();
const char* storageRoot();

bool listRemoteFiles(std::vector<String>& out);
bool loadRemoteProfile(const char* path, RemoteProfile& out);
bool saveRemoteProfile(const RemoteProfile& profile, const char* path);
bool saveLearnedCapture(const IrCapture& cap, const char* category, const char* label, String& outPath);
bool saveLearnedCommand(const RemoteCommand& cmd, const char* transport, const char* category,
                        const char* label, String& outPath);

String categoryLabel(const char* slug);
const char* categorySlug(int index);
int categoryCount();
