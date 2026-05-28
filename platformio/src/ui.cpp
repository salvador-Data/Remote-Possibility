#include "ui.h"

#include <M5Cardputer.h>

void uiHeader(const char* title, const char* subtitle) {
    auto& d = M5Cardputer.Display;
    d.fillScreen(TFT_BLACK);
    d.setTextSize(1);
    d.setTextColor(0xB6DF, TFT_BLACK);
    d.setCursor(4, 4);
    d.print(title);
    d.drawFastHLine(0, 18, d.width(), 0x0083);
    if (subtitle && subtitle[0]) {
        d.setTextColor(TFT_DARKGREY, TFT_BLACK);
        d.setCursor(4, 22);
        d.print(subtitle);
    }
}

void uiFooter(const char* hint) {
    auto& d = M5Cardputer.Display;
    d.setTextColor(TFT_DARKGREY, TFT_BLACK);
    d.setCursor(4, 122);
    d.print(hint);
}

void uiListItem(int row, bool selected, const String& line, int yBase) {
    auto& d = M5Cardputer.Display;
    const int y = yBase + row * 12;
    d.setCursor(4, y);
    if (selected) {
        d.setTextColor(TFT_YELLOW, TFT_BLACK);
        d.print("> ");
    } else {
        d.setTextColor(TFT_WHITE, TFT_BLACK);
        d.print("  ");
    }
    String text = line;
    if (text.length() > 28) text = text.substring(0, 28);
    d.print(text);
}

void uiMessage(const char* title, const String& body, uint16_t color) {
    uiHeader(title);
    auto& d = M5Cardputer.Display;
    d.setTextColor(color, TFT_BLACK);
    d.setCursor(4, 40);
    d.print(body);
    uiFooter(";/. nav  Enter  ` back");
}

void uiProgress(const char* title, int current, int total, const char* label) {
    uiHeader(title, label);
    auto& d = M5Cardputer.Display;
    d.setTextColor(TFT_WHITE, TFT_BLACK);
    d.setCursor(4, 48);
    d.printf("Try %d / %d", current + 1, total);
    d.setCursor(4, 64);
    d.print("y=worked  n=next  `=stop");
}
