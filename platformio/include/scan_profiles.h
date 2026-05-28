#pragma once

#include <Arduino.h>

struct ScanCandidate {
    const char* brand;
    const char* label;
    const char* protocol;
    uint16_t address;
    uint16_t command;
};

struct ScanBrand {
    const char* name;
    const ScanCandidate* candidates;
    int count;
};

struct ScanCategory {
    const char* slug;
    const char* title;
    const ScanBrand* brands;
    int brandCount;
};

int scanCategoryCount();
const ScanCategory* scanCategoryAt(int index);
const ScanBrand* scanBrandAt(const ScanCategory* cat, int index);
const ScanCandidate* scanCandidateAt(const ScanBrand* brand, int index);
