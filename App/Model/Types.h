#pragma once

#include <cstdint>

class Types {
public:
    
    // RunMode

    enum class RunMode : uint8_t {
        Forward,
        Backward,
        Pendulum,
        PingPong,
        Random,
        RandomWalk,
        Last
    };
};