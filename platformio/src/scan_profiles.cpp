#include "scan_profiles.h"

static const ScanCandidate kSamsungTv[] = {
    {"Samsung", "power", "NEC", 0xE0E0, 0x40BF},
    {"Samsung", "vol_up", "NEC", 0xE0E0, 0xE01F},
    {"Samsung", "vol_dn", "NEC", 0xE0E0, 0xD02F},
    {"Samsung", "mute", "NEC", 0xE0E0, 0xF00F},
    {"Samsung", "source", "NEC", 0xE0E0, 0x7E81},
};

static const ScanCandidate kLgTv[] = {
    {"LG", "power", "NEC", 0x04FB, 0xC035},
    {"LG", "vol_up", "NEC", 0x04FB, 0xD02F},
    {"LG", "vol_dn", "NEC", 0x04FB, 0xE01F},
    {"LG", "mute", "NEC", 0x04FB, 0xF00F},
    {"LG", "input", "NEC", 0x04FB, 0xA05F},
};

static const ScanCandidate kGenericTv[] = {
    {"Generic", "power_a", "NEC", 0x00FF, 0x02FD},
    {"Generic", "power_b", "NEC", 0x20DF, 0x10EF},
    {"Generic", "power_c", "NEC", 0x807F, 0x45BA},
};

static const ScanBrand kTvBrands[] = {
    {"Samsung", kSamsungTv, static_cast<int>(sizeof(kSamsungTv) / sizeof(kSamsungTv[0]))},
    {"LG", kLgTv, static_cast<int>(sizeof(kLgTv) / sizeof(kLgTv[0]))},
    {"Generic NEC", kGenericTv, static_cast<int>(sizeof(kGenericTv) / sizeof(kGenericTv[0]))},
};

static const ScanCandidate kGenericFan[] = {
    {"Generic", "power", "NEC", 0x00FF, 0x42BD},
    {"Generic", "speed", "NEC", 0x00FF, 0x02FD},
    {"Generic", "osc", "NEC", 0x00FF, 0x52AD},
    {"Generic", "timer", "NEC", 0x00FF, 0x22DD},
};

static const ScanBrand kFanBrands[] = {
    {"Generic NEC", kGenericFan, static_cast<int>(sizeof(kGenericFan) / sizeof(kGenericFan[0]))},
};

static const ScanCandidate kGenericAc[] = {
    {"Generic", "power", "NEC", 0x10EF, 0x08F7},
    {"Generic", "temp_up", "NEC", 0x10EF, 0x48B7},
    {"Generic", "temp_dn", "NEC", 0x10EF, 0x58A7},
    {"Generic", "mode", "NEC", 0x10EF, 0x38C7},
};

static const ScanBrand kAcBrands[] = {
    {"Generic NEC", kGenericAc, static_cast<int>(sizeof(kGenericAc) / sizeof(kGenericAc[0]))},
};

static const ScanCandidate kGenericProjector[] = {
    {"Generic", "power", "NEC", 0x00FF, 0x02FD},
    {"Generic", "source", "NEC", 0x00FF, 0x52AD},
    {"Generic", "menu", "NEC", 0x00FF, 0x22DD},
};

static const ScanBrand kProjectorBrands[] = {
    {"Generic NEC", kGenericProjector, static_cast<int>(sizeof(kGenericProjector) / sizeof(kGenericProjector[0]))},
};

static const ScanCategory kCategories[] = {
    {"tv", "Television", kTvBrands, static_cast<int>(sizeof(kTvBrands) / sizeof(kTvBrands[0]))},
    {"fan", "Fan", kFanBrands, static_cast<int>(sizeof(kFanBrands) / sizeof(kFanBrands[0]))},
    {"ac", "Air conditioner", kAcBrands, static_cast<int>(sizeof(kAcBrands) / sizeof(kAcBrands[0]))},
    {"projector", "Projector", kProjectorBrands,
     static_cast<int>(sizeof(kProjectorBrands) / sizeof(kProjectorBrands[0]))},
};

int scanCategoryCount() { return static_cast<int>(sizeof(kCategories) / sizeof(kCategories[0])); }

const ScanCategory* scanCategoryAt(int index) {
    if (index < 0 || index >= scanCategoryCount()) return nullptr;
    return &kCategories[index];
}

const ScanBrand* scanBrandAt(const ScanCategory* cat, int index) {
    if (!cat || index < 0 || index >= cat->brandCount) return nullptr;
    return &cat->brands[index];
}

const ScanCandidate* scanCandidateAt(const ScanBrand* brand, int index) {
    if (!brand || index < 0 || index >= brand->count) return nullptr;
    return &brand->candidates[index];
}
