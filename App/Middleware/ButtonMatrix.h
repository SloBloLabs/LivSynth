#pragma once

#include "ShiftRegister.h"
#include "RingBuffer.h"

#define NUM_BUTTONS 4

class ButtonMatrix {
public:
    struct Event {
        enum Action {
            KeyDown,
            KeyUp
        };

        Event() = default;
        Event(Action action, int value) : _action(action), _value(value) {}

        Action action() const { return Action(_action); }
        int value() const { return _value; }

    private:
        uint8_t _action;
        int8_t _value;
    };

    ButtonMatrix(ShiftRegister &shiftRegister);

    void init();

    inline bool buttonState(int index) const {
        return _buttonState[index].state;
    }

    void process();

    inline bool nextEvent(Event &event) {
        if(_events.readable() < 1) {
            return false;
        }
        event = _events.read();
        return true;
    }

private:
    struct ButtonState {
        uint8_t state;
    };

    ShiftRegister &_shiftRegister;
    ButtonState _buttonState[NUM_BUTTONS];

    RingBuffer<Event, 16> _events;
};