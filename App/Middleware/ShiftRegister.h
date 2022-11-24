#pragma once

#include <cstdint>

class ShiftRegister {
public:
    ShiftRegister();

    void init();
    void process();

    inline uint16_t read() const {
        return _input;
    }

    inline void write(uint16_t const data) {
        _output = data;
    }

private:
    uint16_t _input;
    uint16_t _output;
};