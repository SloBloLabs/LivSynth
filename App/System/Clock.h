#pragma once

#include "ClockTimer.h"
#include "Config.h"
#include "MovingAverage.h"
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
    enum class RunState {
        Idle,
        MasterRunning,
        SlaveRunning
    };

    Clock(ClockTimer &timer);

    void init();

    inline RunState runState() const { return _runState; }
    inline bool isIdle() const { return _runState == RunState::Idle; }
    inline bool isRunning() const { return _runState != RunState::Idle; }
    inline uint32_t ppqn() { return _ppqn; }
    inline uint32_t bpm() const { return _runState == RunState::SlaveRunning ? slaveBpm() : masterBpm(); }

    // Master clock control
    void masterStart();
    void masterStop();
    inline float masterBpm() const { return _masterBpm; }
    void setMasterBpm(float bpm);
    inline float slaveBpm() const { return _slaveBpm; }

    // Slave clock control
    void slaveTick();
    void slaveStart();
    void slaveStop();
    void slaveContinue();
    void slaveReset();
    void slaveHandleMidi(uint8_t msg);

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
    void setupSlaveTimer();

    void outputTick(uint32_t tick);
    void outputClock(bool clock);

    static constexpr uint32_t SlaveTimerPeriod = 100; // us -> 3125 bpm

    ClockTimer &_timer;
    RunState _runState;
    IClockObserver::OutputState _outputState;

    volatile uint32_t _tick;
    volatile uint32_t _tickProcessed;

    uint32_t _slaveDivisor;

    uint32_t _elapsedUs;
    float _masterBpm;
    uint32_t _ppqn;

    // slave sync helpers
    uint32_t _lastSlaveTickUs; // time of last call to slaveTick
    uint32_t _slaveTickPeriodUs = 0; // slave tick period time
    uint32_t _slaveSubTicksPending; // number of slave sub ticks pending
    uint32_t _slaveSubTickPeriodUs = 0; // slave sub tick period time
    uint32_t _nextSlaveSubTickUs; // time of next slave sub tick

    float _slaveBpmFiltered = 0.f;
    MovingAverage<float, 4> _slaveBpmAvg;
    float _slaveBpm = 0.f;

    std::list<IClockObserver*> _observerList;
};