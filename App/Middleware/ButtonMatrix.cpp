#include "ButtonMatrix.h"
#include <cstring>

ButtonMatrix::ButtonMatrix(ShiftRegister &shiftRegister) :
    _shiftRegister(shiftRegister)
{}

void ButtonMatrix::init() {
    std::memset(_buttonState, 0, sizeof(_buttonState));
}

void ButtonMatrix::process() {
    _shiftRegister.process();
    
    // Output data

    uint8_t buttonData = _shiftRegister.read(0);

    for(int i = 0; i < 4; ++i) {
        int buttonIndex = i;
        auto &state = _buttonState[i].state;
        bool newState = !(buttonData & (1 << i));
        if(newState != state) {
            state = newState;
            _events.write(Event(state ? Event::KeyDown : Event::KeyUp, buttonIndex));
        }
    }
}