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

    uint16_t buttonData = _shiftRegister.read();

    for(int buttonIndex = 0; buttonIndex < NUM_BUTTONS; ++buttonIndex) {
        auto &state = _buttonState[buttonIndex].state;
        bool newState = (buttonData & (1 << buttonIndex));
        if(newState != state) {
            state = newState;
            _events.write(Event(state ? Event::KeyDown : Event::KeyUp, buttonIndex));
        }
    }
}