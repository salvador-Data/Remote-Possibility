#pragma once

#include <Arduino.h>
#include <M5Cardputer.h>

void uiHeader(const char* title, const char* subtitle = nullptr);
void uiFooter(const char* hint);
void uiListItem(int row, bool selected, const String& line, int yBase = 48);
void uiMessage(const char* title, const String& body, uint16_t color = TFT_WHITE);
void uiProgress(const char* title, int current, int total, const char* label);
