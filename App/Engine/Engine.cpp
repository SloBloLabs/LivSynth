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

void Engine::keyDown(KeyEvent &event) {
    DBG("Engine::keyDown key=%d, count=%d", event.key().code(), event.count());
    //event.key().show();

    NoteSequence &sequence = static_cast<NoteTrackEngine*>(_trackEngine)->sequence();

    if(event.key().isStep()) {
        if(event.key().state(Key::Code::Shift)) {
            if(event.count() > 1) {
                sequence.step(event.key().code()).toggleGate();
            }
        } else {
            setGateOutputOverride(true);
            setCvOutputOverride(true);
            setGateOutput(true);
            _selectedStep = event.key().code();
            setCvOutput(sequence.step(_selectedStep).note());
        }
    } else if(event.key().isPlay()) {
        togglePlay();
    } else if(event.key().isShift()) {
        // TODO: implement shifted modes
        
    }
    
}

void Engine::keyUp(KeyEvent &event) {
    DBG("Engine::keyUp   key=%d, count=%d", event.key().code(), event.count());
    //event.key().show();

    if(event.key().isStep()) {
        setGateOutput(false);
        updateOverrides();
        updatePeripherals();
    } else if(event.key().none()) {
        setGateOutputOverride(false);
        setCvOutputOverride(false);
    }
}

void Engine::setCV(PotEvent &event) {
    if(event.index() == 0) {
        // Pitch
        if(_selectedStep != -1) {
            NoteSequence &sequence = static_cast<NoteTrackEngine*>(_trackEngine)->sequence();
            //DBG("Set Pitch:%.2f", event.value());
            sequence.step(_selectedStep).setNote(event.value() * 0xFFF);
            
            if(gateOutputOverride()) {
                setCvOutput(sequence.step(_selectedStep).note());
            }
        }
    } else if(event.index() == 1) {
        // Tempo
        /*
        0 - 4095 ^= 20 - 300 bpm
        0    -> 20
        4095 -> 300
        f(x) = mx + b = dy/dx * x + b = (300 - 20) / 4095 * x + b
        f(0) = 280/4095 * 0 + b = 20
        f(x) = 280/4096 * x + 20
        */
        float bpm =  280.f * event.value() + 20.f;
        //DBG("New BPM:%.2f", bpm);
        _model.project().setTempo(bpm);
    }
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
    quantizeCV(cvOutput);
    dac.setValue(cvOutput);
    dio.setGate(gateOutput);
}

void Engine::updateOverrides() {
    if(_gateOutputOverride) {
        dio.setGate(_gateOverrideValue);
    }
    if(_cvOutputOverride) {
        quantizeCV(_cvOverrideValue);
        dac.setValue(_cvOverrideValue);
    }
}

void Engine::updatePeripherals() {
    dac.update();
    dio.update();
}

void Engine::quantizeCV(uint32_t &cvValue) {
    // semitones
    float delta = 4096.f / 61; // 5 Octaves * 12 semitones + 1 last C
    uint8_t k = floorf(cvValue / delta + .5f);
    cvValue = k * delta;
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