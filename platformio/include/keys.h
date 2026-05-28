#pragma once

#include <M5Cardputer.h>

struct Keys {
    bool up = false;
    bool down = false;
    bool ok = false;
    bool back = false;
    bool yes = false;
    bool no = false;
    bool send = false;
    bool learn = false;
    bool scan = false;
    bool refresh = false;
    bool backspace = false;
    char typed = 0;
};

inline Keys readKeys() {
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
        if (key == 'y' || key == 'Y') k.yes = true;
        if (key == 'n' || key == 'N') k.no = true;
        if (key == 'x' || key == 'X') k.send = true;
        if (key == 'l' || key == 'L') k.learn = true;
        if (key == 'f' || key == 'F') k.scan = true;
        if (key == 'r' || key == 'R') k.refresh = true;
        if (key >= 32 && key <= 126 && key != '\n') k.typed = static_cast<char>(key);
    }
    if (status.del) k.backspace = true;
    return k;
}
