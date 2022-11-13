#pragma once

#include "TimeSignature.h"
#include "ClockSetup.h"
#include "NoteTrack.h"

#include "Math.h"

class Project {
public:
    Project();

    float tempo() const { return _tempo; }
    void setTempo(float tempo) {
        _tempo = clamp(tempo, 1.f, 1000.f);
    }

    uint8_t swing() const { return _swing; };
    void setSwing(uint8_t swing) {
        _swing = clamp(swing, uint8_t(50), uint8_t(75));
    }

    TimeSignature timeSignature() const { return _timeSignature; }
    void setTimeSignature(TimeSignature timeSignature) {
        _timeSignature = timeSignature;
    }

    const ClockSetup &clockSetup() const { return _clockSetup; }
          ClockSetup &clockSetup()       { return _clockSetup; }
    
    const NoteTrack &track() const { return _track; }
          NoteTrack &track()       { return _track; }
    
    const NoteSequence &noteSequence(int patternIndex) const { return _track.sequence(patternIndex); }
          NoteSequence &noteSequence(int patternIndex)       { return _track.sequence(patternIndex); }
    
    void clear();

private:
    float _tempo;
    uint8_t _swing;
    TimeSignature _timeSignature;
    ClockSetup _clockSetup;
    NoteTrack _track;
};