#include <cstdint>
#include "Math.h"

namespace Groove {

// The second and fourth sixteenth notes of each beat are delayed.
// The higher the parameter is set, the more the beats in question are delayed.
static uint32_t applySwing(uint32_t tick, int swing, uint32_t base = CONFIG_PPQN / 4) {
    uint32_t period = 2 * base;

    uint32_t beat = tick / period;
    tick %= period;

    if (tick <= base) {
        tick = remap(tick, base, (period * swing) / 100);
    } else {
        tick = period - remap(period - tick, base, (period * (100 - swing)) / 100);
    }

    return beat * period + tick;
}

} // namespace Groove