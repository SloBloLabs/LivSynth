#include "ClockTimer.h"
#include "main.h"

#include <algorithm>

#define CLOCK_TIMER TIM5

void ClockTimer::init() {
    _period = 0;
}

void ClockTimer::enable() {
    // Enable TIM2 update event interrupt (Call TIM5_IRQHandler)
    LL_TIM_EnableIT_UPDATE(CLOCK_TIMER);
    LL_TIM_EnableCounter(CLOCK_TIMER);
}

void ClockTimer::disable() {
    LL_TIM_DisableIT_UPDATE(CLOCK_TIMER);
    LL_TIM_DisableCounter(CLOCK_TIMER);
}

bool ClockTimer::isRunning() {
    return LL_TIM_IsEnabledCounter(CLOCK_TIMER);
}

void ClockTimer::setPeriod(uint32_t us) {
    _period = us;
    LL_TIM_SetAutoReload(CLOCK_TIMER, _period - 1);
    LL_TIM_SetCounter(CLOCK_TIMER, std::min(LL_TIM_GetCounter(CLOCK_TIMER), _period - 1));
}

// Called from TIM update ISR
void ClockTimer::notifyTimerUpdate() {
    // notify observers
    notifyObservers();
}