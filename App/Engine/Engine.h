#pragma once

#include "Model.h"
#include "Project.h"
#include "Clock.h"
#include "AdcInternal.h"
#include "DacInternal.h"
#include "Dio.h"
#include "TrackEngine.h"
#include "Event.h"
#include "MidiHandler.h"

class Engine : private IClockObserver {
public:
    Engine(Model &model, ClockTimer& clockTimer, MidiHandler &midiHandler);
    void init();
    bool update();

    void togglePlay();
    void clockStart();
    void clockStop();
    bool clockRunning();
    void updateClockSetup();

    // tempo
    float tempo() const { return _clock.bpm(); }
    Clock::RunState runState() const { return _clock.runState(); }

    inline uint32_t tick() const { return _tick; }
    uint32_t noteDivisor() const;
    uint32_t measureDivisor() const;

    // gate overrides
    inline bool gateOutputOverride() const { return _gateOutputOverride; }
    inline void setGateOutputOverride(bool enabled) { _gateOutputOverride = enabled; }
    inline void setGateOutput(bool gate) { _gateOverrideValue = gate; }

    // cv overrides
    inline bool cvOutputOverride() const { return _cvOutputOverride; }
    inline void setCvOutputOverride(bool enabled) { _cvOutputOverride = enabled; }
    inline void setCvOutput(float value) { _cvOverrideValue = value; }

    inline void setSelectedStep(int step) { _selectedStep = step; }
    inline int selectedStep() { return _selectedStep; }

    inline TrackEngine* trackEngine() {
        return _trackEngine;
    }

    void updateOverrides();
    void updatePeripherals();

private:
    virtual void onClockOutput(const IClockObserver::OutputState& state) override;
    virtual void onStart() override;
    virtual void onStop() override;

    void updateTrackSetup();
    void updateTrackOutputs();
    uint32_t quantizeCV(uint32_t cvValue);

    void receiveMidi();
    //void receiveMidi(MidiPort port, uint8_t cable, const MidiMessage &message);

    void initClock();
    
    Model &_model;
    Project &_project;
    
    Clock _clock;
    MidiHandler &_midiHandler;

    TrackEngine* _trackEngine;

    uint32_t _tick = 0;

    // gate output overrides
    bool _gateOutputOverride = false;
    bool _gateOverrideValue = 0;

    // cv output overrides
    bool _cvOutputOverride = false;
    uint32_t _cvOverrideValue = 0;

    int _selectedStep = 0;
};