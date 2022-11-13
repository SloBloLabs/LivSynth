#include "Types.h"
#include "Random.h"
#include <cstdint>

class SequenceState {
public:
    int step() const { return _step; }
    int prevStep() const { return _prevStep; }
    int direction() const { return _direction; }
    uint32_t iteration() const { return _iteration; }

    void reset();

    void advcanceFree(Types::RunMode runMode, int firstStep, int lastStep, Random &rnd);
    void advanceAligned(int absoluteStep, Types::RunMode runMode, int firstStep, int lastStep, Random &rnd);

private:
    void advanceRandomWalk(int firstStep, int lastStep, Random &rnd);
    
    int8_t _step;
    int8_t _prevStep;
    int8_t _direction;
    uint32_t _iteration;
};