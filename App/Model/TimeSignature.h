#pragma once

#include "Config.h"
#include "Math.h"

#include <array>
#include <cstdint>

class TimeSignature {
public:
    TimeSignature() = default;

    TimeSignature(int beats, int note) {
        setNote(note);
        setBeats(beats);
    }

    int beats() const { return _beats; }
    void setBeats(int beats) {
        _beats = clamp(beats, 1, _note * 2);
    }

    int note() const { return _note; }
    void setNote(int note) {
        auto it = std::find(validNotes.begin(), validNotes.end(), uint8_t(note));
        if(it == validNotes.end()) {
            _note = DefaultNote;
        } else {
            _note = *it;
        }
        setBeats(beats());
    }

    uint32_t noteDivisor() const {
        return (CONFIG_PPQN * 4) / note();
    }

    uint32_t measureDivisor() const {
        return beats() * noteDivisor();
    }

private:
    static const std::array<uint8_t, 6> validNotes;
    static constexpr int DefaultBeats = 4;
    static constexpr int DefaultNote = 4;

    uint8_t _beats = DefaultBeats;
    uint8_t _note = DefaultNote;
};