#pragma once

#include <cstdint>
#include <array>

#define NUM_SHIFT_REGISTERS 1

class ShiftRegister {
public:
    ShiftRegister();

    void init();
    void process();

    uint8_t read(int index) const {
        return _inputs[index];
    }

    void write(int index, uint8_t value) {
        _outputs[index];
    }

private:
    std::array<uint8_t, NUM_SHIFT_REGISTERS> _outputs;
    std::array<uint8_t, NUM_SHIFT_REGISTERS> _inputs;
};