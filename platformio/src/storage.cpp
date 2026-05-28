#include "storage.h"

#include <SD.h>
#include <SPI.h>

static bool gReady = false;
static String gRoot = kRemotesPrimary;

static const char* kCatSlugs[] = {"tv", "fan", "ac", "projector", "other"};
static const char* kCatLabels[] = {"Television", "Fan", "Air conditioner", "Projector", "Other"};

int categoryCount() { return static_cast<int>(sizeof(kCatSlugs) / sizeof(kCatSlugs[0])); }

const char* categorySlug(int index) {
    if (index < 0 || index >= categoryCount()) return "other";
    return kCatSlugs[index];
}

String categoryLabel(const char* slug) {
    if (!slug) return "Other";
    for (int i = 0; i < categoryCount(); ++i) {
        if (strcmp(slug, kCatSlugs[i]) == 0) return kCatLabels[i];
    }
    return "Other";
}

bool storageInit() {
    SPI.begin(kSdSclk, kSdMiso, kSdMosi, kSdCs);
    if (!SD.begin(kSdCs, SPI, 25000000)) {
        gReady = false;
        return false;
    }
    gReady = true;
    if (SD.exists(kRemotesPrimary)) {
        gRoot = kRemotesPrimary;
    } else {
        gRoot = kRemotesLegacy;
        if (!SD.exists(kRemotesLegacy)) {
            SD.mkdir(kRemotesLegacy);
        }
    }
    return true;
}

bool storageReady() { return gReady; }

const char* storageRoot() { return gRoot.c_str(); }

static bool endsWithRemote(const String& name) {
    return name.endsWith(".remote") || name.endsWith(".ir") || name.endsWith(".json");
}

bool listRemoteFiles(std::vector<String>& out) {
    out.clear();
    if (!gReady) return false;
    File dir = SD.open(gRoot.c_str());
    if (!dir || !dir.isDirectory()) return false;
    while (true) {
        File entry = dir.openNextFile();
        if (!entry) break;
        if (!entry.isDirectory()) {
            String name = entry.name();
            if (endsWithRemote(name)) {
                out.push_back(String(gRoot) + "/" + name);
            }
        }
        entry.close();
    }
    dir.close();
    return true;
}

static void trim(String& s) { s.trim(); }

static bool parseHex16(const String& s, uint16_t& out) {
    String t = s;
    trim(t);
    if (t.startsWith("0x") || t.startsWith("0X")) t = t.substring(2);
    out = static_cast<uint16_t>(strtoul(t.c_str(), nullptr, 16));
    return true;
}

bool loadRemoteProfile(const char* path, RemoteProfile& out) {
    if (!gReady || !path) return false;
    File f = SD.open(path, FILE_READ);
    if (!f) return false;
    out = RemoteProfile{};
    out.path = path;
    RemoteCommand* cmd = nullptr;
    while (f.available()) {
        String line = f.readStringUntil('\n');
        line.replace("\r", "");
        trim(line);
        if (!line.length() || line.startsWith("#")) continue;
        if (line.startsWith("[")) {
            RemoteCommand c;
            int end = line.indexOf(']');
            c.label = (end > 1) ? line.substring(1, end) : "cmd";
            out.commands.push_back(c);
            cmd = &out.commands.back();
            continue;
        }
        int eq = line.indexOf('=');
        if (eq < 1) continue;
        String key = line.substring(0, eq);
        String val = line.substring(eq + 1);
        trim(key);
        trim(val);
        key.toLowerCase();
        if (key == "name") out.name = val;
        else if (key == "category") out.category = val;
        else if (key == "brand") out.brand = val;
        else if (key == "transport") out.transport = val;
        else if (key == "protocol") out.protocol = val;
        else if (key == "address") parseHex16(val, out.address);
        else if (cmd) {
            if (key == "label") cmd->label = val;
            else if (key == "protocol") cmd->protocol = val;
            else if (key == "address") parseHex16(val, cmd->address);
            else if (key == "command") parseHex16(val, cmd->command);
            else if (key == "raw") {
                cmd->hasRaw = true;
                cmd->rawLen = 0;
                int start = 0;
                while (start < val.length() && cmd->rawLen < kMaxRawLen) {
                    int comma = val.indexOf(',', start);
                    String part = (comma < 0) ? val.substring(start) : val.substring(start, comma);
                    trim(part);
                    if (part.length()) {
                        cmd->raw[cmd->rawLen++] = static_cast<uint16_t>(part.toInt());
                    }
                    if (comma < 0) break;
                    start = comma + 1;
                }
            }
        }
    }
    f.close();
    if (!out.name.length()) {
        int slash = out.path.lastIndexOf('/');
        out.name = (slash >= 0) ? out.path.substring(slash + 1) : out.path;
    }
    return true;
}

bool saveRemoteProfile(const RemoteProfile& profile, const char* path) {
    if (!gReady || !path) return false;
    File f = SD.open(path, FILE_WRITE);
    if (!f) return false;
    f.printf("name=%s\n", profile.name.c_str());
    f.printf("category=%s\n", profile.category.c_str());
    f.printf("brand=%s\n", profile.brand.c_str());
    f.printf("transport=%s\n", profile.transport.c_str());
    f.printf("protocol=%s\n", profile.protocol.c_str());
    f.printf("address=0x%04X\n", profile.address);
    for (const auto& cmd : profile.commands) {
        f.printf("\n[%s]\n", cmd.label.c_str());
        f.printf("protocol=%s\n", cmd.protocol.c_str());
        f.printf("address=0x%04X\n", cmd.address);
        f.printf("command=0x%04X\n", cmd.command);
        if (cmd.hasRaw && cmd.rawLen > 0) {
            f.print("raw=");
            for (uint16_t i = 0; i < cmd.rawLen; ++i) {
                if (i) f.print(",");
                f.print(cmd.raw[i]);
            }
            f.print("\n");
        }
    }
    f.close();
    return true;
}

bool saveLearnedCapture(const IrCapture& cap, const char* category, const char* label, String& outPath) {
    if (!gReady) return false;
    RemoteProfile p;
    p.name = label ? label : "learned";
    p.category = category ? category : "other";
    p.brand = "Learned";
    p.transport = "ir";
    p.protocol = cap.protocol;
    p.address = cap.address;
    RemoteCommand c;
    c.label = "power";
    c.protocol = cap.protocol;
    c.address = cap.address;
    c.command = cap.command;
    if (cap.rawLen > 0) {
        c.hasRaw = true;
        c.rawLen = cap.rawLen;
        memcpy(c.raw, cap.raw, cap.rawLen * sizeof(uint16_t));
    }
    p.commands.push_back(c);
    const unsigned long n = millis() % 10000;
    outPath = String(gRoot) + "/" + String(category ? category : "other") + "_" +
              String(n) + ".remote";
    return saveRemoteProfile(p, outPath.c_str());
}

bool saveLearnedCommand(const RemoteCommand& cmd, const char* transport, const char* category,
                        const char* label, String& outPath) {
    if (!gReady) return false;
    RemoteProfile p;
    p.name = label ? label : "learned";
    p.category = category ? category : "other";
    p.brand = "Learned";
    p.transport = transport ? transport : "ir";
    p.protocol = cmd.protocol;
    p.address = cmd.address;
    p.commands.push_back(cmd);
    const unsigned long n = millis() % 10000;
    outPath = String(gRoot) + "/" + String(category ? category : "other") + "_" + String(n) +
              ".remote";
    return saveRemoteProfile(p, outPath.c_str());
}
