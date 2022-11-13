#pragma once

#include "Math.h"

namespace ModelUtils {

template<typename Enum>
static Enum clampedEnum(Enum value) {
    return Enum(clamp(int(value), 0, int(Enum::Last) - 1));
}

int divisorToIndex(int divisor);
int indexToDivisor(int index);

} // namespace ModelUtils