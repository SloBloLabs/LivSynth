#include "Engine.h"
#include "Config.h"
#include "System.h"
#include "NoteTrackEngine.h"
#include "swvPrint.h"
#include "Math.h"
#include <cmath>

extern Dio dio;
extern DacInternal dac;

Engine::Engine(Model &model, ClockTimer& clockTimer, MidiHandler &midiHandler) :
    _model(model),
    _project(model.project()),
    _clock(clockTimer),
    _midiHandler(midiHandler)
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
    _clock.setMasterBpm(_project.tempo());

    updateClockSetup();

    // consume midi events
    receiveMidi();

    uint32_t tick;
    bool outputUpdated = false;
    while(_clock.checkTick(&tick)) {
        _tick = tick;

        bool updated = _trackEngine->tick(tick);
        if(updated) {
            _trackEngine->update();
            updateTrackOutputs();
            updateOverrides();
            outputUpdated = true;
        }
    }
    
    if(outputUpdated) {
        _trackEngine->update();
        updateTrackOutputs();
        updateOverrides();
        updatePeripherals();
    } else if(!clockRunning()) {
        updateOverrides();
        updatePeripherals();
    }

    _midiHandler.processOutgoing();

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
    onStart();
    _clock.masterStart();
}

void Engine::clockStop() {
    // flush track engine
    // may solve performer issue #345?
    _trackEngine->tick(UINT32_MAX - 1);
    _trackEngine->update();
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

void Engine::onStart() {
    _midiHandler.setBusy(false);
}

void Engine::onStop() {
    clockStop();
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
    if(!dio.gate() && gateOutput) {
        // generate note on
        //UDBG("NoteOn\n");
        uint8_t midiNote = static_cast<uint8_t>(floorf(61.f/4096.f * cvOutput)) + 36; // 3 octaves up
        MidiMessage msg = MidiMessage::makeNoteOn(0, midiNote);
        _midiHandler.enqueueOutgoing(msg);
    } else if(dio.gate() && !gateOutput) {
        // generate note off
        //UDBG("NoteOff\n");
        uint8_t midiNote = static_cast<uint8_t>(floorf(61.f/4096.f * cvOutput)) + 36; // 3 octaves up
        MidiMessage msg = MidiMessage::makeNoteOff(0, midiNote);
        _midiHandler.enqueueOutgoing(msg);
    }
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
    //float delta = 4095.f / 61; // 5 Octaves * 12 semitones + 1 last C
    //uint8_t k = floorf(cvValue / delta);
    uint32_t cvqValue = quantize(cvValue, 4096.f, 61.f);// 5 Octaves * 12 semitones + 1 last C
    //DBG("CV_org: %ld, CV_q: %ld", cvValue, cvqValue);
    return cvqValue;
}

void Engine::receiveMidi() {
    MidiMessage msg;
    while(_midiHandler.dequeueIncoming(&msg)) {
        if(MidiMessage::isChannelMessage(msg.status())) {
            //UDBG("Channel Message\n");
            // TODO: play note etc.
        }
    }
}

void Engine::initClock() {
    _clock.attach(this);

    // called by USB ISR
    _midiHandler.setRealtimeHandler([this] (MidiMessage &msg) {
        if(msg.isClockMessage()) {
            _clock.slaveHandleMidi(msg.status());
            return true;
        }
        return false;
    });
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