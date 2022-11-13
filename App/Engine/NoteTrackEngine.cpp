#include "NoteTrackEngine.h"
#include "Config.h"
#include "Engine.h"
#include "Groove.h"
#include "Random.h"

static Random rnd;

void NoteTrackEngine::reset() {
    _freeRelativeTick = 0;
    _sequenceState.reset();
    _currentStep = -1;
    _activity = false;
    _gateOutput = false;
    _cvOutput = 0.f;

    changePattern();
}

bool NoteTrackEngine::tick(uint32_t tick) {
    if(_sequence == nullptr)
        return false;

    const auto &sequence = *_sequence;

    uint32_t divisor = sequence.divisor() * (CONFIG_PPQN / CONFIG_SEQUENCE_PPQN);
    uint32_t resetDivisor = sequence.resetMeasure() * _engine.measureDivisor();
    uint32_t relativeTick = resetDivisor == 0 ? tick : tick % resetDivisor;
    
    // hanlde reset measure
    if(relativeTick == 0) {
        reset();
    }

    // advance sequence, play mode: aligned
    if(relativeTick % divisor == 0) {
        _sequenceState.advanceAligned(relativeTick / divisor, sequence.runMode(), sequence.firstStep(), sequence.lastStep(), rnd);
        triggerStep(tick, divisor);
    }

    bool updateResult = false;

    while(!_gateQueue.empty() && tick >= _gateQueue.front().tick) {
        // update gates
        updateResult = true;
        _activity = _gateQueue.front().gate;
        _gateOutput = _activity;
        _gateQueue.pop();
    }

    while(!_cvQueue.empty() && tick >= _cvQueue.front().tick) {
        // update CV
        updateResult = true;
        _cvOutputTarget = _cvQueue.front().cv;
        _cvQueue.pop();
    }

    return updateResult;
}

void NoteTrackEngine::update(float dt) {
    //TODO: update Gate and CV
    _cvOutput = _cvOutputTarget;
}

void NoteTrackEngine::changePattern() {
    _sequence = &_noteTrack.sequence(pattern());
}

void NoteTrackEngine::triggerStep(uint32_t tick, uint32_t divisor) {
    const auto &sequence = *_sequence;
    _currentStep = _sequenceState.step();
    
    uint32_t gateOffset = 0;

    const auto &step = sequence.step(_currentStep);
    bool stepGate = step.gate();

    if(stepGate) {
        uint32_t stepLength = (divisor * step.length()) / NoteSequence::Length::Range;
        // Gate
        _gateQueue.pushReplace({ Groove::applySwing(tick + gateOffset             , swing()), true });
        _gateQueue.pushReplace({ Groove::applySwing(tick + gateOffset + stepLength, swing()), false });

        // CV
        uint32_t note = step.note();
        _cvQueue.push({ Groove::applySwing(tick + gateOffset, swing()), note });
    }
    _triggerStep = true;
}