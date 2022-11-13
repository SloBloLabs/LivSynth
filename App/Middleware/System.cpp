#include "System.h"
#include "main.h"

volatile uint32_t System::_ticks;

void System::init() {
    // Configure and enable Systick timer including interrupt
    SysTick_Config((SystemCoreClock / 1000) - 1);
}

void System::tick() {
    ++_ticks;
}
