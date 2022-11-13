#include "ModelUtils.h"
#include "KnownDivisor.h"

int divisorToIndex(int divisor) {
    for (int i = 0; i < numKnownDivisors; ++i) {
        const auto &knownDivisor = knownDivisors[i];
        if (divisor == knownDivisor.divisor) {
            return knownDivisor.index;
        }
    }
    return -1;
}

int indexToDivisor(int index) {
    for (int i = 0; i < numKnownDivisors; ++i) {
        const auto &knownDivisor = knownDivisors[i];
        if (index == knownDivisor.index) {
            return knownDivisor.divisor;
        }
    }
    return -1;
}