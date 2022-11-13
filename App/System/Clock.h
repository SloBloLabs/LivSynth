#pragma once

#include "ClockTimer.h"
#include "Config.h"
#include <list>

class IClockObserver {
public:
    struct OutputState {
        bool clock = false;
        bool reset = true;
        bool run = false;
    };
    virtual ~IClockObserver() {}
    virtual void onClockOutput(const OutputState &state) = 0;
    // virtual void onClockMidi(uint8_t) = 0;
};

class IClock {
public:
    virtual ~IClock() {}
    virtual void attach(IClockObserver* observer) = 0;
    virtual void detach(IClockObserver* observer) = 0;
    virtual void notifyObservers() = 0;
};

class Clock : public IClockTimerObserver, public IClock  {
public:

    Clock(ClockTimer &timer);

    void init();

    inline bool isIdle() const { return _runState == RunState::Idle; }
    inline bool isRunning() const { return _runState != RunState::Idle; }
    inline uint32_t ppqn() { return _ppqn; }
    inline uint32_t bpm() const { return masterBpm(); }

    // Master clock control
    void masterStart();
    void masterStop();
    inline float masterBpm() const { return _masterBpm; }
    void setMasterBpm(float bpm);

    uint32_t tick() const { return _tick; }
    inline float tickDuration() const { return 60.f / (bpm() * _ppqn); }

    inline void attach(IClockObserver* observer) override {
        _observerList.push_back(observer);
    }

    inline void detach(IClockObserver* observer) override {
        _observerList.remove(observer);
    }

    // Clock output
    void outputConfigure(int divisor, int pulse);
    void outputConfigureSwing(int swing);

    inline const IClockObserver::OutputState &outputState() const { return _outputState; }
    
    // Sequencer interface
    bool checkTick(uint32_t* tick);

    void onClockTimerTick() override;

private:
    enum class RunState {
        Idle,
        MasterRunning
    };

    struct Output {
        int divisor;
        int pulse;
        int swing;
        uint32_t nextTick;
        uint32_t nextTickOn;
        uint32_t nextTickOff;
    } _output;

    inline void notifyObservers() override {
        for(auto observer : _observerList) {
            observer->onClockOutput(_outputState);
        }
    }

    void resetTicks();
    inline void setRunState(RunState runState) { _runState = runState; }

    void setupMasterTimer();

    void outputTick(uint32_t tick);
    void outputClock(bool clock);

    ClockTimer &_timer;
    RunState _runState;
    IClockObserver::OutputState _outputState;

    volatile uint32_t _tick;
    volatile uint32_t _tickProcessed;

    uint32_t _elapsedUs;
    float _masterBpm;
    uint32_t _ppqn;

    std::list<IClockObserver*> _observerList;
};