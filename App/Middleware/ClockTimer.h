#pragma once

#include <cstdint>
#include <list>

// https://refactoring.guru/design-patterns/observer/cpp/example
// https://interrupt.memfault.com/blog/cortex-m-fault-debug

class IClockTimerObserver {
public:
    virtual ~IClockTimerObserver() {}
    virtual void onClockTimerTick() = 0;
};

class IClockTimer {
public:
    virtual ~IClockTimer() {}
    virtual void attach(IClockTimerObserver* observer) = 0;
    virtual void detach(IClockTimerObserver* observer) = 0;
    virtual void notifyObservers() = 0;
};

class ClockTimer : public IClockTimer {
public:
    void init();

    void enable();
    void disable();
    bool isRunning();

    inline uint32_t period() const { return _period; }
    void setPeriod(uint32_t us);

    inline void attach(IClockTimerObserver* observer) override {
        _observerList.push_back(observer);
    }

    inline void detach(IClockTimerObserver* observer) override {
        _observerList.remove(observer);
    }

    void notifyTimerUpdate();

private:
    inline void notifyObservers() override {
        for(auto observer : _observerList) {
            observer->onClockTimerTick();
        }
    }

    uint32_t _period;
    std::list<IClockTimerObserver*> _observerList;
};
