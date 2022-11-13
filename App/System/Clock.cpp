#include "Clock.h"
#include "main.h"
#include "Groove.h"
#include <cmath>

Clock::Clock(ClockTimer &timer) :
    _timer(timer)
{
    resetTicks();
    _ppqn = CONFIG_PPQN;
    _masterBpm = 120.f;
    _runState = RunState::Idle;
    _timer.attach(this);
}

void Clock::init() {
    _timer.disable();
}

void Clock::masterStart() {
    setRunState(RunState::MasterRunning);
    resetTicks();

    _timer.disable();
    setupMasterTimer();
    _timer.enable();
}

void Clock::masterStop() {
    setRunState(RunState::Idle);
    _timer.disable();
}

void Clock::resetTicks() {
    _tick = 0;
    _tickProcessed = 0;
    _output.nextTick = 0;
}

void Clock::setMasterBpm(float bpm) {
    _masterBpm = bpm;
    setupMasterTimer();
}

// outputDivisor * (_clock.ppqn() / SEQUENCE_PPQN), outputPulse
// outputDivisor = 12 -> 1/16 note
// ppqn = 192
// SEQUENCE_PPQN = 48
// divisor = 12 * (192 / 48) = 12 * 4 = 48
// outputPulse = 1
/*
Divisor	Note	Name
2	1/64T	Triplet sixty-fourth note
3	1/64	Sixty-fourth note
4	1/32T	Triplet thirty-second note
6	1/32	Thirty-second note
8	1/16T	Triplet sixteenth note
9	1/32.	Dotted thirty-second note
12	1/16	Sixteenth note
16	1/8T	Triplet eighth note
18	1/16.	Dotted sixteenth note
24	1/8	    Eighth note
32	1/4T	Triplet quarter note
36	1/8.	Dotted eighth note
48	1/4	    Quarter note
64	1/2T	Triplet half note
72	1/4.	Dotted quarter note
96	1/2	    Half note
128	1T	    Triplet whole note
144	1/2.	Dotted half note
192	1	    Whole note
256	2T	    Triplet double note
288	1.	    Dotted whole note
384	2	    Double note
512	4T	    Triplet quadruple note
576	2.	    Dotted double note
768	4	    Quadruple note
 -> see Model/KnownDivisor.h
*/
void Clock::outputConfigure(int divisor, int pulse) {
    _output.divisor = divisor;
    _output.pulse = pulse;
}

void Clock::outputConfigureSwing(int swing) {
    _output.swing = swing;
}

bool Clock::checkTick(uint32_t* tick) {
    if(_tickProcessed < _tick) {
        *tick = _tickProcessed++;
        return true;
    }
    return false;
}

 // called from ClockTimer ISR
void Clock::onClockTimerTick() {
    //DBG("Clock Timer Tick");
    switch(_runState) {
    case RunState::MasterRunning: {
        outputTick(_tick);
        ++_tick;
        _elapsedUs += _timer.period();
        break;
    }
    default:
        break;
    }
}

void Clock::setupMasterTimer() {
    _elapsedUs = 0;
    // round() reduces errors in period calculation, especially for bpm > 120 where only 1 us can determine the tempo on the tenths place.
    // From 181 onwards the difference in period can be zero from one tenth bpm to the next.
    uint32_t us = std::round((60 * 1000000) / (_masterBpm * _ppqn));
    _timer.setPeriod(us);
}

// called from onClockTimerTick
void Clock::outputTick(uint32_t tick) {

    auto applySwing = [this] (uint32_t tick) {
        return _output.swing != 0 ? Groove::applySwing(tick, _output.swing) : tick;
    };

    if(tick == _output.nextTick) {
        uint32_t divisor = _output.divisor;
        // "at least 1 tick long (e.g. 2,604ms @ 120 bpm), the second term kicks in at bpm > 312,5 if pulse equals 1"
        uint32_t clockDuration = std::max(uint32_t(1), uint32_t(_masterBpm * _ppqn * _output.pulse / (60 * 1000)));

        _output.nextTickOn = applySwing(_output.nextTick);
        _output.nextTickOff = std::min(_output.nextTickOn + clockDuration, applySwing(_output.nextTick + divisor) - 1);
        
        _output.nextTick += divisor;
    }

    if(tick == _output.nextTickOn) {
        outputClock(true);
    }

    if(tick == _output.nextTickOff) {
        outputClock(false);
    }
}

void Clock::outputClock(bool clock) {
    if(clock != _outputState.clock) {
        _outputState.clock = clock;
        notifyObservers();
    }
}