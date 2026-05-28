/**
 * Remote Possibility — universal IR remote for M5 Cardputer
 * Hacker Planet LLC · Salvador Data · authorized home/lab use only
 *
 * Home: ;/. move · Enter select · ` back
 * Remote: x send · l learn add · f scan library
 * Learn: point OEM remote · Enter saves to SD
 * Scan: y=worked n=next · saves winning code
 */

#include <M5Cardputer.h>

#include "config.h"
#include "ir_service.h"
#include "keys.h"
#include "remote_transport.h"
#include "scan_profiles.h"
#include "storage.h"
#include "ui.h"

enum class Screen {
    Home,
    RemoteList,
    RemoteDetail,
    LearnPick,
    LearnWait,
    ScanCategory,
    ScanBrand,
    ScanTry,
    About,
};

static Screen gScreen = Screen::Home;
static int gMenuSel = 0;
static int gListSel = 0;
static int gScroll = 0;

static std::vector<String> gRemotePaths;
static RemoteProfile gProfile;
static int gCmdSel = 0;

static int gCatSel = 0;
static int gBrandSel = 0;
static int gTryIdx = 0;
static const ScanCategory* gScanCat = nullptr;
static const ScanBrand* gScanBrand = nullptr;
static RemoteCommand gLearnCmd;
static bool gLearnGot = false;
static Transport gLearnTransport = Transport::Ir;
static int gLearnPickSel = 0;

static const char* kHomeItems[] = {"My remotes", "Learn remote", "Scan device (IR)", "About"};
static const int kHomeCount = 4;

static void drawHome() {
    uiHeader("Remote Possibility", "Hacker Planet LLC");
    for (int i = 0; i < kHomeCount; ++i) {
        uiListItem(i, i == gMenuSel, kHomeItems[i]);
    }
    uiFooter(";/. nav  Enter  ` — SD optional");
}

static void drawRemoteList() {
    uiHeader("My remotes", storageReady() ? storageRoot() : "No SD — scan only");
    if (!storageReady()) {
        uiMessage("SD not mounted", "Insert microSD for save/learn.\nScan still works in RAM.");
        return;
    }
    if (gRemotePaths.empty()) {
        uiMessage("No remotes", "Learn (l) or Scan (f) to add.\nFiles: *.remote on SD");
        return;
    }
    const int start = gScroll;
    for (int row = 0; row < kVisibleRows; ++row) {
        const int idx = start + row;
        if (idx >= static_cast<int>(gRemotePaths.size())) break;
        String name = gRemotePaths[idx];
        const int slash = name.lastIndexOf('/');
        if (slash >= 0) name = name.substring(slash + 1);
        uiListItem(row, idx == gListSel, name);
    }
    uiFooter(";/. list  Enter open  ` home");
}

static void sendCommand(const RemoteCommand& cmd) {
    transportSend(cmd, transportFromSlug(gProfile.transport.c_str()));
}

static void drawRemoteDetail() {
    uiHeader(gProfile.name.c_str(), categoryLabel(gProfile.category.c_str()).c_str());
    if (gProfile.commands.empty()) {
        uiMessage(gProfile.name.c_str(), "No commands in profile.");
        return;
    }
    const int start = gScroll;
    for (int row = 0; row < kVisibleRows; ++row) {
        const int idx = start + row;
        if (idx >= static_cast<int>(gProfile.commands.size())) break;
        uiListItem(row, idx == gCmdSel, gProfile.commands[idx].label);
    }
    uiFooter("x=send  ;/. cmd  ` back");
}

static void drawLearnPick() {
    uiHeader("Learn remote", "Pick transport");
    for (int i = 0; i < static_cast<int>(Transport::Count); ++i) {
        const auto t = static_cast<Transport>(i);
        String line = transportLabel(t);
        if (!transportReady(t)) line += " (module)";
        uiListItem(i, i == gLearnPickSel, line);
    }
    uiFooter(";/. nav  Enter  ` home");
}

static void drawLearn() {
    uiHeader("Learn", transportLabel(gLearnTransport));
    auto& d = M5Cardputer.Display;
    d.setTextColor(TFT_WHITE, TFT_BLACK);
    d.setCursor(4, 48);
    d.print("Press OEM remote btn…");
    d.setCursor(4, 88);
    d.setTextColor(TFT_DARKGREY, TFT_BLACK);
    d.print("15s timeout · ` cancel");
    uiFooter("Enter saves to SD");
}

static void drawScanCategory() {
    uiHeader("Scan device", "Pick category");
    for (int i = 0; i < scanCategoryCount() && i < kVisibleRows; ++i) {
        const ScanCategory* c = scanCategoryAt(i);
        if (!c) continue;
        uiListItem(i, i == gCatSel, c->title);
    }
    uiFooter(";/. nav  Enter  ` home");
}

static void drawScanBrand() {
    if (!gScanCat) return;
    uiHeader("Scan device", gScanCat->title);
    for (int i = 0; i < gScanCat->brandCount && i < kVisibleRows; ++i) {
        const ScanBrand* b = scanBrandAt(gScanCat, i);
        if (!b) continue;
        uiListItem(i, i == gBrandSel, b->name);
    }
    uiFooter(";/. brand  Enter  ` back");
}

static void drawScanTry() {
    if (!gScanBrand) return;
    const ScanCandidate* cand = scanCandidateAt(gScanBrand, gTryIdx);
    if (!cand) return;
    uiProgress("Trying code", gTryIdx, gScanBrand->count, cand->label);
}

static void drawAbout() {
    uiHeader("About", "Salvador Data");
    auto& d = M5Cardputer.Display;
    d.setTextColor(TFT_WHITE, TFT_BLACK);
    d.setCursor(4, 40);
    d.println("Hacker Planet LLC");
    d.println("Philadelphia, PA");
    d.println("GitHub: salvador-Data");
    d.println("Reddit: u/SalvadorData");
    d.setTextColor(TFT_DARKGREY, TFT_BLACK);
    d.setCursor(4, 88);
    d.println("IR + RF + CC1101");
    d.println("Devices you own.");
    uiFooter("` home");
}

static void redraw() {
    switch (gScreen) {
        case Screen::Home:
            drawHome();
            break;
        case Screen::RemoteList:
            drawRemoteList();
            break;
        case Screen::RemoteDetail:
            drawRemoteDetail();
            break;
        case Screen::LearnPick:
            drawLearnPick();
            break;
        case Screen::LearnWait:
            drawLearn();
            break;
        case Screen::ScanCategory:
            drawScanCategory();
            break;
        case Screen::ScanBrand:
            drawScanBrand();
            break;
        case Screen::ScanTry:
            drawScanTry();
            break;
        case Screen::About:
            drawAbout();
            break;
    }
}

static void refreshRemoteList() {
    gRemotePaths.clear();
    if (storageReady()) listRemoteFiles(gRemotePaths);
    gListSel = 0;
    gScroll = 0;
}

static void navList(int& sel, int& scroll, int count, bool up, bool down) {
    if (up) sel = (sel <= 0) ? count - 1 : sel - 1;
    if (down) sel = (sel + 1) % count;
    if (sel < scroll) scroll = sel;
    if (sel >= scroll + kVisibleRows) scroll = sel - kVisibleRows + 1;
}

static void startScanTry() {
    gTryIdx = 0;
    gScreen = Screen::ScanTry;
    redraw();
    const ScanCandidate* cand = scanCandidateAt(gScanBrand, gTryIdx);
    if (cand && strcmp(cand->protocol, "NEC") == 0) {
        irSendNec(cand->address, cand->command);
    }
}

void setup() {
    auto cfg = M5.config();
    M5Cardputer.begin(cfg, true);
    M5Cardputer.Display.setRotation(1);
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.fillScreen(TFT_BLACK);
    M5Cardputer.Display.setCursor(4, 50);
    M5Cardputer.Display.setTextColor(0xB6DF, TFT_BLACK);
    M5Cardputer.Display.print("Remote Possibility");
    M5Cardputer.Display.setTextColor(TFT_DARKGREY, TFT_BLACK);
    M5Cardputer.Display.setCursor(4, 66);
    M5Cardputer.Display.print("Hacker Planet LLC");

    Serial.begin(115200);
    delay(300);
    Serial.println(F("Remote Possibility — IR/RF/sub-GHz"));

    transportInitAll();
    storageInit();
    refreshRemoteList();

    gScreen = Screen::Home;
    gMenuSel = 0;
    redraw();
}

void loop() {
    M5Cardputer.update();
    const Keys k = readKeys();

    if (gScreen == Screen::LearnWait) {
        if (!gLearnGot) {
            RemoteCommand cmd;
            if (transportLearn(gLearnTransport, 80, cmd)) {
                gLearnCmd = cmd;
                gLearnGot = true;
                uiMessage("Captured", String(cmd.protocol) + " · Enter=save", TFT_GREEN);
            }
        } else if (k.ok) {
            String path;
            if (storageReady() &&
                saveLearnedCommand(gLearnCmd, transportSlug(gLearnTransport), "other", "learned",
                                   path)) {
                uiMessage("Saved", path, TFT_GREEN);
                delay(1200);
                refreshRemoteList();
            } else {
                uiMessage("No SD", "Insert SD to save profile.", TFT_YELLOW);
                delay(1200);
            }
            gLearnGot = false;
            gScreen = Screen::Home;
            redraw();
        }
    }

    switch (gScreen) {
        case Screen::Home:
            if (k.back) break;
            if (k.up || k.down) {
                navList(gMenuSel, gScroll, kHomeCount, k.up, k.down);
                redraw();
            }
            if (k.ok) {
                if (gMenuSel == 0) {
                    refreshRemoteList();
                    gScreen = Screen::RemoteList;
                } else if (gMenuSel == 1) {
                    gLearnGot = false;
                    gLearnPickSel = 0;
                    gScreen = Screen::LearnPick;
                } else if (gMenuSel == 2) {
                    gCatSel = 0;
                    gScreen = Screen::ScanCategory;
                } else {
                    gScreen = Screen::About;
                }
                redraw();
            }
            break;

        case Screen::RemoteList:
            if (k.back) {
                gScreen = Screen::Home;
                redraw();
                break;
            }
            if (!storageReady() || gRemotePaths.empty()) {
                if (k.back || k.scan) {
                    gCatSel = 0;
                    gScreen = Screen::ScanCategory;
                    redraw();
                }
                break;
            }
            if (k.up || k.down) {
                navList(gListSel, gScroll, static_cast<int>(gRemotePaths.size()), k.up, k.down);
                redraw();
            }
            if (k.ok && gListSel < static_cast<int>(gRemotePaths.size())) {
                if (loadRemoteProfile(gRemotePaths[gListSel].c_str(), gProfile)) {
                    gCmdSel = 0;
                    gScroll = 0;
                    gScreen = Screen::RemoteDetail;
                    redraw();
                }
            }
            break;

        case Screen::RemoteDetail:
            if (k.back) {
                gScreen = Screen::RemoteList;
                redraw();
                break;
            }
            if (k.send && !gProfile.commands.empty()) {
                sendCommand(gProfile.commands[gCmdSel]);
            }
            if (k.up || k.down) {
                navList(gCmdSel, gScroll, static_cast<int>(gProfile.commands.size()), k.up, k.down);
                redraw();
            }
            break;

        case Screen::LearnPick:
            if (k.back) {
                gScreen = Screen::Home;
                redraw();
                break;
            }
            if (k.up || k.down) {
                navList(gLearnPickSel, gScroll, static_cast<int>(Transport::Count), k.up, k.down);
                redraw();
            }
            if (k.ok) {
                gLearnTransport = static_cast<Transport>(gLearnPickSel);
                gLearnGot = false;
                gScreen = Screen::LearnWait;
                redraw();
            }
            break;

        case Screen::LearnWait:
            if (k.back) {
                gLearnGot = false;
                gScreen = Screen::LearnPick;
                redraw();
            }
            break;

        case Screen::ScanCategory:
            if (k.back) {
                gScreen = Screen::Home;
                redraw();
                break;
            }
            if (k.up || k.down) {
                navList(gCatSel, gScroll, scanCategoryCount(), k.up, k.down);
                redraw();
            }
            if (k.ok) {
                gScanCat = scanCategoryAt(gCatSel);
                gBrandSel = 0;
                gScreen = Screen::ScanBrand;
                redraw();
            }
            break;

        case Screen::ScanBrand:
            if (k.back) {
                gScreen = Screen::ScanCategory;
                redraw();
                break;
            }
            if (!gScanCat) break;
            if (k.up || k.down) {
                navList(gBrandSel, gScroll, gScanCat->brandCount, k.up, k.down);
                redraw();
            }
            if (k.ok) {
                gScanBrand = scanBrandAt(gScanCat, gBrandSel);
                startScanTry();
            }
            break;

        case Screen::ScanTry:
            if (k.back) {
                gScreen = Screen::ScanBrand;
                redraw();
                break;
            }
            if (k.yes && gScanBrand && storageReady()) {
                const ScanCandidate* cand = scanCandidateAt(gScanBrand, gTryIdx);
                if (cand) {
                    RemoteProfile p;
                    p.name = String(gScanCat->title) + " " + gScanBrand->name;
                    p.category = gScanCat->slug;
                    p.brand = gScanBrand->name;
                    p.transport = "ir";
                    p.protocol = cand->protocol;
                    p.address = cand->address;
                    RemoteCommand c;
                    c.label = cand->label;
                    c.protocol = cand->protocol;
                    c.address = cand->address;
                    c.command = cand->command;
                    p.commands.push_back(c);
                    String path = String(storageRoot()) + "/" + p.category + "_scan.remote";
                    saveRemoteProfile(p, path.c_str());
                    uiMessage("Saved", path, TFT_GREEN);
                    delay(1200);
                    refreshRemoteList();
                }
                gScreen = Screen::Home;
                redraw();
                break;
            }
            if (k.no || k.down) {
                if (gScanBrand && gTryIdx + 1 < gScanBrand->count) {
                    ++gTryIdx;
                    redraw();
                    const ScanCandidate* cand = scanCandidateAt(gScanBrand, gTryIdx);
                    if (cand && strcmp(cand->protocol, "NEC") == 0) {
                        irSendNec(cand->address, cand->command);
                    }
                } else {
                    gScreen = Screen::ScanBrand;
                    redraw();
                }
            }
            if (k.up && gScanBrand && gTryIdx > 0) {
                --gTryIdx;
                redraw();
                const ScanCandidate* cand = scanCandidateAt(gScanBrand, gTryIdx);
                if (cand && strcmp(cand->protocol, "NEC") == 0) {
                    irSendNec(cand->address, cand->command);
                }
            }
            break;

        case Screen::About:
            if (k.back) {
                gScreen = Screen::Home;
                redraw();
            }
            break;
    }

    delay(35);
}
