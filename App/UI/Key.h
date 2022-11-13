#pragma once

#include "Config.h"

#include <bitset>

typedef std::bitset<CONFIG_NUM_BUTTONS> KeyState;

class Key {
public:
    // key codes
    enum Code {
        None = -1,
        Step1 = 0,
        Step2 = 1,
        Step3 = 2,
        Step4 = 3,
        Step5 = 4,
        Step6 = 5,
        Step7 = 6,
        Step8 = 7,
        Play  = 8,
        Shift = 9
    };

    Key(int code, const KeyState &state) : _code(code), _state(state) {}

    int code() const { return _code; }

    const KeyState &state() const { return _state; }
    bool state(int code) const { return _state[code]; }

    bool is(int code) const { return _code == code; }

    bool isPlay() const { return is(Play); }
    bool isShift() const { return is(Shift); }

    bool isStep() const { return _code >= Step1 && _code <= Step8; }

    bool none() const { return _state.none(); }

private:
    int _code;
    const KeyState &_state;
};