#pragma once

#include "Model.h"
#include "Project.h"
#include "Clock.h"
#include "AdcInternal.h"
#include "DacInternal.h"
#include "Dio.h"
#include "TrackEngine.h"
#include "Event.h"

class Engine : private IClockObserver {
public:
    Engine(Model &model, ClockTimer& clockTimer);
    void init();
    bool update();

    void togglePlay();
    void clockStart();
    void clockStop();
    bool clockRunning();
    void updateClockSetup();

    inline uint32_t tick() const { return _tick; }
    uint32_t noteDivisor() const;
    uint32_t measureDivisor() const;

    // gate overrides
    inline bool gateOutputOverride() const { return _gateOutputOverride; }
    inline void setGateOutputOverride(bool enabled) { _gateOutputOverride = enabled; }
    inline void setGateOutput(bool gate) { _gateOutputOverrideValue = gate; }

    // cv overrides
    inline bool cvOutputOverride() const { return _cvOutputOverride; }
    inline void setCvOutputOverride(bool enabled) { _cvOutputOverride = enabled; }
    inline void setCvOutput(float value) { _cvOverrideValue = value; }

    inline void setSelectedStep(int step) { _selectedStep = step; }
    inline int selectedStep() { return _selectedStep; }

    inline TrackEngine* trackEngine() {
        return _trackEngine;
    }

    // event handlers
    void keyDown(KeyEvent &event);
    void keyUp(KeyEvent &event);
    void setCV(PotEvent &event);

private:
    virtual void onClockOutput(const IClockObserver::OutputState& state) override;

    void updateTrackSetup();
    void updateTrackOutputs();
    void updateOverrides();

    void initClock();
    
    Model &_model;
    Project &_project;
    
    Clock _clock;

    TrackEngine* _trackEngine;

    uint32_t _tick = 0;
    uint32_t _lastSystemTicks = 0;

    // gate output overrides
    bool _gateOutputOverride = false;
    bool _gateOutputOverrideValue = 0;

    // cv output overrides
    bool _cvOutputOverride = false;
    float _cvOverrideValue = 0;

    int _selectedStep = -1;
};