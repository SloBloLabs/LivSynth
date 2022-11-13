#include "Config.h"
#include "RingBuffer.h"

class CVReader {
public:
    void init();

    void push(size_t index, uint16_t value);
    bool pull(size_t index, uint16_t &value);

    float getCV(size_t index);

private:
    RingBuffer<uint16_t, 50> _potInputBuffer[CONFIG_NUM_POTS];

    float _potValueLp1[CONFIG_NUM_POTS];
    float _sampleBase = 1 / 4095.f;
};