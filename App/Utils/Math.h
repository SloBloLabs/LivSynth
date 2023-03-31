#pragma once

#include <cstdint>
#include <cmath>
#include <algorithm>

template<typename T>
static T clamp(T value, T min, T max) {
    return std::max(min, std::min(max, value));
}

template<typename T>
static T quantize(T value, float inputRange, float outputRange) {
    float delta = inputRange / outputRange;
    uint8_t k = floorf(value / delta);
    T qValue = k * delta;
    return qValue;
}