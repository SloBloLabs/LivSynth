#pragma once

#include "Model.h"
#include "NoteTrack.h"
#include "EnumUtils.h"

class Engine;

class TrackEngine {
public:

    TrackEngine(Engine &engine, const Model &model, NoteTrack &track) :
        _engine(engine),
        _model(model),
        _track(track)
    {
        _pattern = 0;
        changePattern();
    }

    // sequencer control
    virtual void reset() = 0;
    virtual bool tick(uint32_t tick) = 0;
    virtual void update(float dt) = 0;
    virtual void changePattern() {}

    // track output
    virtual bool activity() const = 0;
    virtual bool gateOutput() const = 0;
    virtual float cvOutput() const = 0;

    virtual float sequenceProgress() const { return -1.f; }

    int swing() const { return _model.project().swing(); }
    
    inline uint8_t pattern() const { return _pattern; }
    inline void setPattern(uint8_t pattern) {
        _pattern = pattern;
    }

protected:
    Engine &_engine;
    const Model &_model;
    NoteTrack &_track;
    uint8_t _pattern;
};