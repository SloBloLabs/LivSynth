#include "NoteSequence.h"

int NoteSequence::layerDefaultValue(Layer layer) {

    NoteSequence::Step step;

    switch(layer) {
    case Layer::Gate:
        return step.gate();
    case Layer::Length:
        return step.length();
    case Layer::Note:
        return step.note();
    case Layer::Last:
        break;
    }

    return 0;
}

void NoteSequence::Step::clear() {
    _data0.raw = 0;
    setGate(false);
    setLength(Length::Max / 2);
    setNote(0);
}

void NoteSequence::clear() {
    setDivisor(12);
    setResetMeasure(0);
    setRunMode(Types::RunMode::Forward);
    setFirstStep(0);
    setLastStep(7);

    clearSteps();
}

void NoteSequence::clearSteps() {
    for(auto &step : _steps) {
        step.clear();
    }
}

void NoteSequence::setGates(std::initializer_list<int> gates) {
    size_t step = 0;
    for(auto gate : gates) {
        if(step < _steps.size()) {
            _steps[step++].setGate(gate);
        }
    }
}

void NoteSequence::setNotes(std::initializer_list<int> notes) {
    size_t step = 0;
    for(auto note : notes) {
        if(step < _steps.size()) {
            _steps[step++].setNote(note);
        }
    }
}