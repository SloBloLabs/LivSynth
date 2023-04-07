#pragma once

#include "ModelUtils.h"
#include "Math.h"

#include <cstdint>

class ClockSetup {
public:
    enum class Mode : uint8_t {
        Auto = 0,
        Master,
        Slave,
        Last
    };

    Mode mode() const { return _mode; }
    void setMode(Mode mode) {
        mode = ModelUtils::clampedEnum(mode);
        if(mode != _mode) {
            _mode = mode;
            setDirty();
        }
    }

    uint8_t clockOutputDivisor() const { return _clockOutputDivisor; }
    void setClockOutputDivisor(uint8_t clockOutputDivisor) {
        clockOutputDivisor = clamp(clockOutputDivisor, uint8_t(2), uint8_t(192));
        if(clockOutputDivisor != _clockOutputDivisor) {
            _clockOutputDivisor = clockOutputDivisor;
            setDirty();
        }
    }

    bool clockOutputSwing() const { return _clockOutputSwing; }
    void setClockOutputSwing(bool clockOutputSwing) {
        if(clockOutputSwing != _clockOutputSwing) {
            _clockOutputSwing = clockOutputSwing;
            setDirty();
        }
    }

    uint8_t clockOutputPulse() const { return _clockOutputPulse; }
    void setClockOutputPulse(uint8_t clockOutputPulse) {
        clockOutputPulse = clamp(clockOutputPulse, uint8_t(1), uint8_t(20));
        if(clockOutputPulse != _clockOutputPulse) {
            _clockOutputPulse = clockOutputPulse;
            setDirty();
        }
    }

    void clear();

    bool isDirty() const { return _dirty; }
    void setDirty() { _dirty = true; }
    void clearDirty() { _dirty = false; }

private:
    Mode _mode;
    uint8_t _clockOutputDivisor;
    bool _clockOutputSwing;
    uint8_t _clockOutputPulse;
    bool _dirty;
};