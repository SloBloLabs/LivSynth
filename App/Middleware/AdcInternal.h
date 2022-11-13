#pragma once

#include <cstdint>

#define NUM_ADC_CHANNELS 2

class AdcInternal {
public:
    void init();

    inline uint16_t channel(int index) {
        return _channels[index];
    }

    inline volatile uint16_t *channels() {
        return _channels;
    }

private:
    volatile uint16_t _channels[NUM_ADC_CHANNELS];
};