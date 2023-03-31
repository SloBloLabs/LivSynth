#include "Engine.h"
#include "Config.h"
#include "System.h"
#include "NoteTrackEngine.h"
#include "swvPrint.h"
#include <cmath>

extern Dio dio;
extern DacInternal dac;

Engine::Engine(Model &model, ClockTimer& clockTimer) :
    _model(model),
    _project(model.project()),
    _clock(clockTimer)
{
    _trackEngine = nullptr;
}

void Engine::init() {
    _clock.init();
    initClock();
    updateTrackSetup();
}

// called from main loop each ms
bool Engine::update() {
    uint32_t systemTicks = System::ticks();
    float dt = (0.001f * (systemTicks - _lastSystemTicks));
    _lastSystemTicks = systemTicks;

    _clock.setMasterBpm(_project.tempo());

    updateClockSetup();

    uint32_t tick;
    bool outputUpdated = false;
    while(_clock.checkTick(&tick)) {
        _tick = tick;

        bool updated = _trackEngine->tick(tick);
        if(updated) {
            _trackEngine->update(0.f);
            updateTrackOutputs();
            updateOverrides();
            outputUpdated = true;
        }
    }
    
    if(outputUpdated) {
        _trackEngine->update(dt);
        updateTrackOutputs();
        updateOverrides();
        updatePeripherals();
    } else if(!clockRunning()) {
        updateOverrides();
        updatePeripherals();
    }

    return outputUpdated || static_cast<NoteTrackEngine*>(_trackEngine)->stepTriggered();
}

void Engine::togglePlay() {
    if(clockRunning()) {
        clockStop();
    } else {
        clockStart();
    }
}

void Engine::clockStart() {
    updateClockSetup();
    _clock.masterStart();
}

void Engine::clockStop() {
    // flush track engine
    // may solve performer issue #345?
    _trackEngine->tick(UINT32_MAX - 1);
    _trackEngine->update(0.f);
    updateTrackOutputs();
    updateOverrides();
    updatePeripherals();

    _clock.masterStop();
}

bool Engine::clockRunning() {
    return _clock.isRunning();
}

uint32_t Engine::noteDivisor() const {
    return _project.timeSignature().noteDivisor();
}

uint32_t Engine::measureDivisor() const {
    return _project.timeSignature().measureDivisor();
}

// called by Clock::notifyObservers
void Engine::onClockOutput(const IClockObserver::OutputState& state) {
    dio.setClock(state.clock);
    dio.setReset(state.reset);
}

void Engine::updateTrackSetup() {
    if(!_trackEngine) {
        _trackEngine = new NoteTrackEngine(*this, _model, _project.track());
    }
}

void Engine::updateTrackOutputs() {
    uint32_t cvOutput = _trackEngine->cvOutput();
    bool gateOutput = _trackEngine->gateOutput();
    //DBG("Ticks: %ld: Progress: %.2f, Gate: %d, CV: %.2f", _lastSystemTicks, _trackEngine->sequenceProgress(), gateOutput, cvOutput);
    uint32_t cvqOutput = quantizeCV(cvOutput);
    dac.setValue(cvqOutput);
    dio.setGate(gateOutput);
}

void Engine::updateOverrides() {
    if(_gateOutputOverride) {
        dio.setGate(_gateOverrideValue);
    }
    if(_cvOutputOverride) {
        uint32_t cvqOverrideValue = quantizeCV(_cvOverrideValue);
        dac.setValue(cvqOverrideValue);
    }
}

void Engine::updatePeripherals() {
    dac.update();
    dio.update();
}

uint32_t Engine::quantizeCV(uint32_t cvValue) {
    // semitones
    float delta = 4095.f / 61; // 5 Octaves * 12 semitones + 1 last C
    uint8_t k = floorf(cvValue / delta);
    uint32_t cvqValue = k * delta;
    //DBG("CV_org: %ld, CV_q: %ld", cvValue, cvqValue);
    return cvqValue;
}

void Engine::initClock() {
    _clock.attach(this);
}

void Engine::updateClockSetup() {
    auto &clockSetup = _project.clockSetup();

    _clock.outputConfigureSwing(clockSetup.clockOutputSwing() ? _project.swing() : 0);

    if(!clockSetup.isDirty()) {
        return;
    }
    
    _clock.outputConfigure(clockSetup.clockOutputDivisor() * (CONFIG_PPQN / CONFIG_SEQUENCE_PPQN), clockSetup.clockOutputPulse());

    onClockOutput(_clock.outputState());

    clockSetup.clearDirty();
}