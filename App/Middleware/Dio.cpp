#include "Dio.h"
#include "main.h"

void Dio::setClock(bool clock) {
    _clock = clock;
    if(_clock) {
        LL_GPIO_SetOutputPin(CLOCK_OUT_GPIO_Port, CLOCK_OUT_Pin);
    } else {
        LL_GPIO_ResetOutputPin(CLOCK_OUT_GPIO_Port, CLOCK_OUT_Pin);
    }
}

void Dio::setReset(bool reset) {
    _reset = reset;
    if(_reset) {
        LL_GPIO_SetOutputPin(RESET_OUT_GPIO_Port, RESET_OUT_Pin);
    } else {
        LL_GPIO_ResetOutputPin(RESET_OUT_GPIO_Port, RESET_OUT_Pin);
    }
}

void Dio::setGate(bool gate) {
    _gate = gate;
}

void Dio::update() {
    if(_gate) {
        LL_GPIO_SetOutputPin(GATE_OUT_GPIO_Port, GATE_OUT_Pin);
    } else {
        LL_GPIO_ResetOutputPin(GATE_OUT_GPIO_Port, GATE_OUT_Pin);
    }
}