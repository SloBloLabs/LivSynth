#pragma once

#include "Bitfield.h"
#include "Types.h"
#include "ModelUtils.h"

#include <cstdint>
#include <array>

#define CONFIG_STEP_COUNT 8

class NoteSequence {
public:
    typedef UnsignedValue<3> Length;
    typedef UnsignedValue<12> Note;

    enum class Layer {
        Gate,
        Length,
        Note,
        Last
    };

    static int layerDefaultValue(Layer layer);

    class Step {
    public:
        inline bool gate() const { return _data0.gate ? true : false; }
        inline void setGate(bool gate) { _data0.gate = gate; }
        inline void toggleGate() { setGate(!gate()); }

        inline int length() const { return _data0.length; }
        inline void setLength(int length) {
            _data0.length = Length::clamp(length);
        }

        inline uint32_t note() const { return Note::Min + _data0.note; }
        inline void setNote(uint32_t note) {
            _data0.note = Note::clamp(note) - Note::Min;
        }

        Step() { clear(); }
        void clear();
        
    private:
        union {
            uint32_t raw;
            BitField<uint32_t, 0, 1> gate;
            BitField<uint32_t, 1, Length::Bits> length;
            BitField<uint32_t, 4, Note::Bits> note;
            // 16 bits left
        } _data0;
    };

    typedef std::array<Step, CONFIG_STEP_COUNT> StepArray;

    inline int divisor() const { return _divisor; }
    inline void setDivisor(int divisor) {
        _divisor = divisor;
    }

    uint8_t resetMeasure() const { return _resetMeasure; }
    void setResetMeasure(uint8_t resetMeasure) {
        _resetMeasure = clamp(resetMeasure, uint8_t(0), uint8_t(128));
    }

    inline int indexedDivisor() const { return ModelUtils::divisorToIndex(divisor()); }
    inline void setIndexedDivisor(int index) {
        int divisor = ModelUtils::indexToDivisor(index);
        if(divisor > 0) {
            setDivisor(divisor);
        }
    }

    inline Types::RunMode runMode() const { return _runMode; }
    inline void setRunMode(Types::RunMode runMode) {
        _runMode = runMode;
    }

    inline uint8_t firstStep() const { return _firstStep; }
    inline void setFirstStep(uint8_t firstStep) {
        _firstStep = firstStep;
    }

    inline uint8_t lastStep() const { return _lastStep; }
    inline void setLastStep(uint8_t lastStep) {
        _lastStep = lastStep;
    }

    inline const StepArray &steps() const { return _steps; }
    inline       StepArray &steps()       { return _steps; }

    inline const Step &step(int index) const { return _steps[index]; }
    inline       Step &step(int index)       { return _steps[index]; }

    NoteSequence() { clear(); }

    void clear();
    void clearSteps();

    void setGates(std::initializer_list<int> gates);
    void setNotes(std::initializer_list<int> notes);

private:
    uint16_t _divisor;
    uint8_t _resetMeasure;
    Types::RunMode _runMode;
    uint8_t _firstStep;
    uint8_t _lastStep;

    StepArray _steps;
};
