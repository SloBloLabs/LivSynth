#include "ClockSetup.h"

void ClockSetup::clear() {
    _mode = Mode::Auto;
    _clockOutputDivisor = 12;
    _clockOutputSwing = false;
    _clockOutputPulse = 1; 
    _dirty = true;
}