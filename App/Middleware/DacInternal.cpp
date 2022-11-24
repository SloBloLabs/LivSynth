#include "DacInternal.h"
#include "main.h"

#define DAC_VOCT DAC1
#define DAC_VOCT_CHANNEL LL_DAC_CHANNEL_1
#define DAC_DELAY_VOLTAGE_SETTLING_CYCLES 29

void DacInternal::init() {
    _value = 0x0;
    // Start DAC
    LL_DAC_ConvertData12RightAligned(DAC_VOCT, DAC_VOCT_CHANNEL, _value);
    LL_DAC_Enable(DAC_VOCT, DAC_VOCT_CHANNEL);
    volatile uint32_t wait_loop_index = ((LL_DAC_DELAY_STARTUP_VOLTAGE_SETTLING_US * (SystemCoreClock / (100000 * 2))) / 10);
    while(wait_loop_index != 0) {
        wait_loop_index--;
    }
    LL_DAC_EnableTrigger(DAC_VOCT, DAC_VOCT_CHANNEL);
}

void DacInternal::setValue(uint32_t value) {
    _value = value;
}

void DacInternal::update() {
    LL_DAC_ConvertData12RightAligned(DAC_VOCT, DAC_VOCT_CHANNEL, _value);
    LL_DAC_TrigSWConversion(DAC_VOCT, DAC_VOCT_CHANNEL);
    volatile uint32_t wait_loop_index = DAC_DELAY_VOLTAGE_SETTLING_CYCLES;
    while(wait_loop_index != 0) {
        wait_loop_index--;
    }
}