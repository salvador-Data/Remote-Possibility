/**
 * Remote Possibility — M5 Cardputer CyberThreatGotchi status client
 *
 * Authorized lab Wi-Fi only. Polls GET /api/status and shows mood + threats.
 *
 * Keyboard (status view): r refresh · s settings · ` back (from settings)
 * Settings: ;/w up · ./s down · Enter edit · ` back to status
 * Edit field: type value · Backspace delete · Enter save · ` cancel
 *
 * Serial (115200): HELP · SHOW · SET HOST|PORT|SSID|PASS|POLL <value> · WIFI
 */

#include <M5Cardputer.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Preferences.h>
#include <ArduinoJson.h>

#include <SD.h>
#include <SPI.h>

#ifndef CTG_HOST
#define CTG_HOST "192.168.1.50"
#endif
#ifndef CTG_PORT
#define CTG_PORT 8765
#endif
#ifndef POLL_MS
#define POLL_MS 4000
#endif

static const char* kNs = "rp_ctg";
static const uint32_t kHttpTimeoutMs = 8000;
static const uint32_t kWifiConnectMs = 25000;
static const uint32_t kWifiRetryMs = 15000;

// M5 OS Cardputer SD SPI (optional config file)
static const int kSdCs = 12;
static const int kSdSclk = 14;
static const int kSdMiso = 39;
static const int kSdMosi = 40;
static const char* kSdCfgPath = "/config/remote_possibility.cfg";

enum class UiMode { Status, Settings, Edit };

enum class SettingItem { Host, Port, Ssid, Pass, Poll, Count };

struct AppConfig {
    String host = CTG_HOST;
    uint16_t port = CTG_PORT;
    String ssid;
    String pass;
    uint32_t pollMs = POLL_MS;
};

struct StatusData {
    String mood = "idle";
    String name = "Cipherhorn";
    int level = 1;
    int blocked = 0;
    int seen = 0;
    String statusLine;
    String lastIp;
    String lastSev;
    String lastAction;
};

static Preferences gPrefs;
static AppConfig gCfg;
static StatusData gLast;
static UiMode gMode = UiMode::Status;
static SettingItem gSettingSel = SettingItem::Host;
static SettingItem gEditItem = SettingItem::Host;
static String gEditBuf;
static bool gFetchOk = false;
static String gWifiLine = "WiFi: init";
static String gHttpLine = "";
static unsigned long gLastPoll = 0;
static unsigned long gLastWifiAttempt = 0;
static bool gForcePoll = false;
static bool gSdCfgLoaded = false;

void processSerial();

static const char* moodLabel(const char* mood) {
    if (!mood) return "IDLE";
    if (strcmp(mood, "happy") == 0) return "OK";
    if (strcmp(mood, "alert") == 0) return "ALERT";
    if (strcmp(mood, "attack") == 0) return "BLOCK";
    if (strcmp(mood, "sleep") == 0) return "ZZZ";
    if (strcmp(mood, "defend") == 0) return "DEF";
    if (strcmp(mood, "feed") == 0) return "FOOD";
    return mood;
}

static uint16_t moodColor(const String& mood) {
    if (mood == "attack") return TFT_RED;
    if (mood == "alert") return TFT_YELLOW;
    if (mood == "happy") return TFT_GREEN;
    if (mood == "sleep") return TFT_BLUE;
    return TFT_WHITE;
}

static void trim(String& s) {
    s.trim();
}

static bool parseKeyValue(const String& line, String& key, String& val) {
    int eq = line.indexOf('=');
    if (eq < 1) return false;
    key = line.substring(0, eq);
    val = line.substring(eq + 1);
    trim(key);
    trim(val);
    key.toLowerCase();
    return key.length() > 0;
}

static void applyCfgKey(const String& key, const String& val) {
    if (key == "host" && val.length()) gCfg.host = val;
    else if (key == "port" && val.length()) gCfg.port = static_cast<uint16_t>(val.toInt());
    else if (key == "ssid" && val.length()) gCfg.ssid = val;
    else if (key == "pass") gCfg.pass = val;
    else if (key == "poll_ms" && val.length()) gCfg.pollMs = static_cast<uint32_t>(val.toInt());
}

static void tryLoadSdConfig() {
    if (gSdCfgLoaded) return;
    gSdCfgLoaded = true;
    SPI.begin(kSdSclk, kSdMiso, kSdMosi, kSdCs);
    if (!SD.begin(kSdCs, SPI, 25000000)) return;
    if (!SD.exists(kSdCfgPath)) return;
    File f = SD.open(kSdCfgPath, FILE_READ);
    if (!f) return;
    while (f.available()) {
        String line = f.readStringUntil('\n');
        line.replace("\r", "");
        trim(line);
        if (!line.length() || line.startsWith("#")) continue;
        String key, val;
        if (parseKeyValue(line, key, val)) applyCfgKey(key, val);
    }
    f.close();
}

static void loadConfig() {
    gPrefs.begin(kNs, false);
    gCfg.host = gPrefs.getString("host", CTG_HOST);
    gCfg.port = static_cast<uint16_t>(gPrefs.getUInt("port", CTG_PORT));
    gCfg.ssid = gPrefs.getString("ssid", WIFI_SSID);
    gCfg.pass = gPrefs.getString("pass", WIFI_PASS);
    gCfg.pollMs = gPrefs.getUInt("poll", POLL_MS);
    if (gCfg.pollMs < 1000) gCfg.pollMs = POLL_MS;
    gPrefs.end();
    tryLoadSdConfig();
}

static void saveConfig() {
    gPrefs.begin(kNs, false);
    gPrefs.putString("host", gCfg.host);
    gPrefs.putUInt("port", gCfg.port);
    gPrefs.putString("ssid", gCfg.ssid);
    gPrefs.putString("pass", gCfg.pass);
    gPrefs.putUInt("poll", gCfg.pollMs);
    gPrefs.end();
}

static const char* settingLabel(SettingItem item) {
    switch (item) {
        case SettingItem::Host: return "CTG host";
        case SettingItem::Port: return "CTG port";
        case SettingItem::Ssid: return "Wi-Fi SSID";
        case SettingItem::Pass: return "Wi-Fi pass";
        case SettingItem::Poll: return "Poll (ms)";
        default: return "?";
    }
}

static String settingValue(SettingItem item) {
    switch (item) {
        case SettingItem::Host: return gCfg.host;
        case SettingItem::Port: return String(gCfg.port);
        case SettingItem::Ssid: return gCfg.ssid.length() ? gCfg.ssid : "(not set)";
        case SettingItem::Pass:
            return gCfg.pass.length() ? "********" : "(not set)";
        case SettingItem::Poll: return String(gCfg.pollMs);
        default: return "";
    }
}

static void startEdit(SettingItem item) {
    gEditItem = item;
    gMode = UiMode::Edit;
    switch (item) {
        case SettingItem::Host: gEditBuf = gCfg.host; break;
        case SettingItem::Port: gEditBuf = String(gCfg.port); break;
        case SettingItem::Ssid: gEditBuf = gCfg.ssid; break;
        case SettingItem::Pass: gEditBuf = gCfg.pass; break;
        case SettingItem::Poll: gEditBuf = String(gCfg.pollMs); break;
        default: gEditBuf = ""; break;
    }
}

static void commitEdit() {
    trim(gEditBuf);
    switch (gEditItem) {
        case SettingItem::Host:
            if (gEditBuf.length()) gCfg.host = gEditBuf;
            break;
        case SettingItem::Port:
            if (gEditBuf.length()) gCfg.port = static_cast<uint16_t>(gEditBuf.toInt());
            break;
        case SettingItem::Ssid:
            gCfg.ssid = gEditBuf;
            break;
        case SettingItem::Pass:
            gCfg.pass = gEditBuf;
            break;
        case SettingItem::Poll: {
            uint32_t ms = static_cast<uint32_t>(gEditBuf.toInt());
            if (ms >= 1000 && ms <= 600000) gCfg.pollMs = ms;
            break;
        }
        default: break;
    }
    saveConfig();
    gMode = UiMode::Settings;
}

static bool wifiConnected() { return WiFi.status() == WL_CONNECTED; }

static void updateWifiLine() {
    if (wifiConnected()) {
        gWifiLine = "WiFi: " + WiFi.localIP().toString();
    } else if (gCfg.ssid.length() == 0) {
        gWifiLine = "WiFi: set SSID (e)";
    } else {
        gWifiLine = "WiFi: connecting…";
    }
}

static bool ensureWifi() {
    updateWifiLine();
    if (wifiConnected()) return true;
    if (gCfg.ssid.length() == 0) return false;
    const unsigned long now = millis();
    if (now - gLastWifiAttempt < 3000) return false;
    gLastWifiAttempt = now;
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    WiFi.begin(gCfg.ssid.c_str(), gCfg.pass.c_str());
    const unsigned long start = millis();
    while (!wifiConnected() && millis() - start < kWifiConnectMs) {
        delay(200);
        M5Cardputer.update();
        processSerial();
    }
    updateWifiLine();
    return wifiConnected();
}

static bool fetchStatus(StatusData& out, String& err) {
    if (!ensureWifi()) {
        err = "no WiFi";
        return false;
    }
    WiFiClient client;
    HTTPClient http;
    client.setTimeout(kHttpTimeoutMs / 1000);
    String url =
        String("http://") + gCfg.host + ":" + String(gCfg.port) + "/api/status";
    if (!http.begin(client, url)) {
        err = "http begin";
        return false;
    }
    http.setTimeout(kHttpTimeoutMs);
    http.setReuse(false);
    const int code = http.GET();
    if (code != HTTP_CODE_OK) {
        err = String("HTTP ") + code;
        http.end();
        return false;
    }
    const String body = http.getString();
    http.end();

    JsonDocument doc;
    if (deserializeJson(doc, body)) {
        err = "json parse";
        return false;
    }
    JsonObject g = doc["gotchi"].as<JsonObject>();
    out.mood = g["mood"] | "idle";
    out.name = g["name"] | "Cipherhorn";
    out.level = g["level"] | 1;
    out.blocked = g["threats_blocked"] | 0;
    out.seen = g["threats_seen"] | 0;
    out.statusLine = g["status_line"] | "";
    JsonArray threats = doc["threats"].as<JsonArray>();
    if (!threats.isNull() && threats.size() > 0) {
        JsonObject t = threats[0].as<JsonObject>();
        out.lastIp = t["source_ip"] | "?";
        out.lastSev = t["severity"] | "";
        out.lastAction = t["action_taken"] | "";
    } else {
        out.lastIp = "";
        out.lastSev = "";
        out.lastAction = "";
    }
    err = "";
    return true;
}

static void drawStatusView() {
    auto& d = M5Cardputer.Display;
    d.fillScreen(TFT_BLACK);
    d.setTextSize(1);
    d.setTextColor(gFetchOk ? TFT_GREEN : TFT_RED);
    d.setCursor(4, 2);
    d.printf("Remote Possibility");
    d.setTextColor(TFT_DARKGREY);
    d.setCursor(4, 14);
    d.print(gWifiLine);
    d.setCursor(4, 26);
    d.printf("%s:%u", gCfg.host.c_str(), gCfg.port);

    if (!gFetchOk) {
        d.setTextColor(TFT_RED);
        d.setCursor(4, 42);
        d.print(gHttpLine.length() ? gHttpLine : "CTG unreachable");
        d.setTextColor(TFT_WHITE);
        d.setCursor(4, 100);
        d.print("r=refresh e=settings");
        return;
    }

    const uint16_t mc = moodColor(gLast.mood);
    d.setTextColor(mc);
    d.setCursor(4, 42);
    d.printf("%s %s Lv%d", gLast.name.c_str(), moodLabel(gLast.mood.c_str()), gLast.level);
    d.setTextColor(TFT_WHITE);
    d.setCursor(4, 56);
    d.printf("Blk:%d See:%d", gLast.blocked, gLast.seen);
    d.setCursor(4, 70);
    String line = gLast.statusLine;
    if (line.length() > 32) line = line.substring(0, 32);
    d.print(line);
    if (gLast.lastIp.length()) {
        d.setTextColor((gLast.mood == "alert" || gLast.mood == "attack") ? TFT_YELLOW : TFT_WHITE);
        d.setCursor(4, 86);
        d.print(gLast.lastIp);
        d.setCursor(4, 100);
        String detail = gLast.lastSev + " " + gLast.lastAction;
        if (detail.length() > 32) detail = detail.substring(0, 32);
        d.print(detail);
    } else {
        d.setCursor(4, 86);
        d.print("No threats yet");
    }
    d.setTextColor(TFT_DARKGREY);
    d.setCursor(4, 118);
    d.print("r=refresh e=settings");
}

static void drawSettingsView() {
    auto& d = M5Cardputer.Display;
    d.fillScreen(TFT_BLACK);
    d.setTextSize(1);
    d.setTextColor(TFT_CYAN);
    d.setCursor(4, 2);
    d.print("Settings (authorized lab)");
    for (int i = 0; i < static_cast<int>(SettingItem::Count); ++i) {
        const auto item = static_cast<SettingItem>(i);
        const int y = 18 + i * 14;
        d.setCursor(4, y);
        if (item == gSettingSel) {
            d.setTextColor(TFT_YELLOW);
            d.print("> ");
        } else {
            d.setTextColor(TFT_WHITE);
            d.print("  ");
        }
        d.printf("%s: ", settingLabel(item));
        String v = settingValue(item);
        if (v.length() > 18) v = v.substring(0, 18);
        d.print(v);
    }
    d.setTextColor(TFT_DARKGREY);
    d.setCursor(4, 118);
    d.print(";/. move Enter edit ` back");
}

static void drawEditView() {
    auto& d = M5Cardputer.Display;
    d.fillScreen(TFT_BLACK);
    d.setTextSize(1);
    d.setTextColor(TFT_CYAN);
    d.setCursor(4, 4);
    d.printf("Edit %s", settingLabel(gEditItem));
    d.setTextColor(TFT_WHITE);
    d.setCursor(4, 24);
    d.print(gEditBuf);
    d.drawRect(4, 38, 232, 12, TFT_DARKGREY);
    d.setTextColor(TFT_DARKGREY);
    d.setCursor(4, 58);
    d.print("Enter=save Backspace=del");
    d.setCursor(4, 72);
    d.print("` cancel");
}

static void redraw() {
    switch (gMode) {
        case UiMode::Status:
            drawStatusView();
            break;
        case UiMode::Settings:
            drawSettingsView();
            break;
        case UiMode::Edit:
            drawEditView();
            break;
    }
}

struct Keys {
    bool up = false;
    bool down = false;
    bool ok = false;
    bool back = false;
    bool refresh = false;
    bool settings = false;
    bool backspace = false;
};

static Keys readKeys() {
    Keys k;
    if (!M5Cardputer.Keyboard.isChange() || !M5Cardputer.Keyboard.isPressed()) {
        return k;
    }
    auto status = M5Cardputer.Keyboard.keysState();
    for (auto key : status.word) {
        if (key == ';' || key == 'w' || key == 'W') k.up = true;
        if (key == '.' || key == 's' || key == 'S') k.down = true;
        if (key == '\n' || key == ' ') k.ok = true;
        if (key == '`' || key == 27) k.back = true;
        if (key == 'r' || key == 'R') k.refresh = true;
        if (key == 'e' || key == 'E') k.settings = true;
    }
    if (status.del) k.backspace = true;
    return k;
}

static void appendEditChar(char c) {
    if (gEditBuf.length() >= 64) return;
    if (c >= 32 && c <= 126) gEditBuf += c;
}

static void handleEditKeys(const Keys& k) {
    if (k.back) {
        gMode = UiMode::Settings;
        redraw();
        return;
    }
    if (k.backspace && gEditBuf.length()) {
        gEditBuf.remove(gEditBuf.length() - 1);
        redraw();
        return;
    }
    if (k.ok) {
        commitEdit();
        redraw();
        return;
    }
    if (!M5Cardputer.Keyboard.isChange() || !M5Cardputer.Keyboard.isPressed()) return;
    auto status = M5Cardputer.Keyboard.keysState();
    for (auto key : status.word) {
        if (key == '\n' || key == ' ' || key == '`') continue;
        appendEditChar(static_cast<char>(key));
    }
    redraw();
}

static void handleSettingsKeys(const Keys& k) {
    if (k.back) {
        gMode = UiMode::Status;
        redraw();
        return;
    }
    if (k.up) {
        int i = static_cast<int>(gSettingSel);
        i = (i <= 0) ? static_cast<int>(SettingItem::Count) - 1 : i - 1;
        gSettingSel = static_cast<SettingItem>(i);
        redraw();
    }
    if (k.down) {
        int i = (static_cast<int>(gSettingSel) + 1) % static_cast<int>(SettingItem::Count);
        gSettingSel = static_cast<SettingItem>(i);
        redraw();
    }
    if (k.ok) {
        startEdit(gSettingSel);
        redraw();
    }
}

static void pollCtg() {
    String err;
    gFetchOk = fetchStatus(gLast, err);
    gHttpLine = err;
    gLastPoll = millis();
    gForcePoll = false;
    if (gMode == UiMode::Status) redraw();
}

static void handleStatusKeys(const Keys& k) {
    if (k.settings) {
        gMode = UiMode::Settings;
        redraw();
        return;
    }
    if (k.refresh) {
        gForcePoll = true;
        pollCtg();
    }
}

static void handleSerialLine(const String& line) {
    String cmd = line;
    trim(cmd);
    if (!cmd.length()) return;
    cmd.toUpperCase();
    if (cmd == "HELP") {
        Serial.println(F("Remote Possibility serial config"));
        Serial.println(F("SHOW | SET HOST <ip> | SET PORT <n> | SET SSID <s>"));
        Serial.println(F("SET PASS <p> | SET POLL <ms> | WIFI | SAVE"));
        return;
    }
    if (cmd == "SHOW" || cmd == "STATUS") {
        Serial.printf("host=%s port=%u poll=%lu ssid=%s\n", gCfg.host.c_str(), gCfg.port,
                        gCfg.pollMs, gCfg.ssid.c_str());
        return;
    }
    if (cmd == "SAVE") {
        saveConfig();
        Serial.println(F("saved"));
        return;
    }
    if (cmd == "WIFI") {
        gLastWifiAttempt = 0;
        ensureWifi();
        updateWifiLine();
        Serial.println(gWifiLine);
        return;
    }
    if (cmd.startsWith("SET ")) {
        int sp1 = cmd.indexOf(' ', 4);
        if (sp1 < 0) return;
        String key = cmd.substring(4, sp1);
        String val = cmd.substring(sp1 + 1);
        trim(val);
        key.toUpperCase();
        if (key == "HOST") gCfg.host = val;
        else if (key == "PORT") gCfg.port = static_cast<uint16_t>(val.toInt());
        else if (key == "SSID") gCfg.ssid = val;
        else if (key == "PASS") gCfg.pass = val;
        else if (key == "POLL") gCfg.pollMs = static_cast<uint32_t>(val.toInt());
        else {
            Serial.println(F("unknown SET key"));
            return;
        }
        saveConfig();
        Serial.println(F("ok"));
        return;
    }
}

static String gSerialBuf;

void processSerial() {
    while (Serial.available()) {
        const char c = static_cast<char>(Serial.read());
        if (c == '\n' || c == '\r') {
            if (gSerialBuf.length()) {
                handleSerialLine(gSerialBuf);
                gSerialBuf = "";
            }
        } else if (gSerialBuf.length() < 120) {
            gSerialBuf += c;
        }
    }
}

void setup() {
    auto cfg = M5.config();
    M5Cardputer.begin(cfg, true);
    M5Cardputer.Display.setRotation(1);
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.fillScreen(TFT_BLACK);
    M5Cardputer.Display.setCursor(4, 50);
    M5Cardputer.Display.setTextColor(TFT_WHITE, TFT_BLACK);
    M5Cardputer.Display.print("Remote Possibility…");

    Serial.begin(115200);
    delay(200);
    Serial.println(F("Remote Possibility — authorized lab CTG client"));
    Serial.println(F("Type HELP for serial SET commands"));

    loadConfig();
    gLastWifiAttempt = 0;
    ensureWifi();
    pollCtg();
    gMode = UiMode::Status;
    redraw();
}

void loop() {
    M5Cardputer.update();
    processSerial();

    if (!wifiConnected() && millis() - gLastWifiAttempt > kWifiRetryMs) {
        gLastWifiAttempt = 0;
        ensureWifi();
        if (gMode == UiMode::Status) redraw();
    }

    const Keys k = readKeys();
    switch (gMode) {
        case UiMode::Status:
            handleStatusKeys(k);
            break;
        case UiMode::Settings:
            if (k.settings) { /* ignore in menu */ }
            handleSettingsKeys(k);
            break;
        case UiMode::Edit:
            handleEditKeys(k);
            break;
    }

    const bool due =
        gForcePoll || (millis() - gLastPoll >= gCfg.pollMs);
    if (due && gMode == UiMode::Status) {
        pollCtg();
    }

    delay(40);
}
