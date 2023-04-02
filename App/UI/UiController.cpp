#include "UiController.h"
#include "System.h"
#include "NoteTrackEngine.h"
#include "swvPrint.h"
#include "Utils.h"
#include <cmath>
#include "Math.h"

extern AdcInternal  adc;
extern ButtonMatrix buttonMatrix;
extern LEDDriver    ledDriver;

void UiController::init() {
    _keyState.reset();
    _lastControllerUpdateTicks = System::ticks();
    _cvReader.init();
    for(size_t i = 0; i < CONFIG_NUM_POTS; ++i) {
        _cvValue[i] = 0.f;
    }
    _pulse = 0.f;
    _uiMode = Perform;
    _storage.read(_model);
}

/*void UiController::update() {
    handleKeys();

    _leds.clear();
    renderSequence();
}*/

void UiController::handleControls(uint32_t time) {
    ButtonMatrix::Event event;
    while(buttonMatrix.nextEvent(event)) {
        bool isDown = event.action() == ButtonMatrix::Event::KeyDown;
        _keyState[event.value()] = isDown;
        Key key(event.value(), _keyState);
        if(isDown) {
            KeyEvent keyPressEvent = _keyPressEventTracker.process(key);
            _keyPressDurationTracker.at(event.value()) = time;
            //DBG("KeyPressEvent type=%d, key=%d, count=%d", keyPressEvent.type(), keyPressEvent.key().code(), keyPressEvent.count());
            handleEvent(keyPressEvent);
        } else {
            uint32_t duration = time - _keyPressDurationTracker.at(event.value());
            KeyEvent keyEvent(KeyEvent::KeyUp, key, 1, duration);
            handleEvent(keyEvent);
        }
    }

    float cvValue;
    for(size_t i = 0; i < CONFIG_NUM_POTS; ++i) {
        cvValue = _cvReader.getCV(i);
        //DBG("%d ov:%.2f, nv:%.2f, diff:%.2f", i, _cvValue[i], cvValue, cvValue - _cvValue[i]);
        if(std::abs(cvValue - _cvValue[i]) > 0.001) {
            // trigger event
            //DBG("cv value updated: index=%d, value=%.2f", i, cvValue);
            _cvValue[i] = cvValue;
            PotEvent potEvent(i, cvValue);
            handleEvent(potEvent);
        }
    }
}

void UiController::renderUI() {
    ledDriver.clear();

    _pulse += .02f;
    if(_pulse >= 1.f) _pulse -= 1.f;

    switch(_uiMode) {
    case Perform:
    {
        uint8_t pattern = _engine.trackEngine()->pattern();
        uint8_t currentStep = reinterpret_cast<NoteTrackEngine*>(_engine.trackEngine())->currentStep();
        NoteSequence &sequence = _model.project().noteSequence(pattern);
        uint8_t firstStep = sequence.firstStep();
        uint8_t lastStep = sequence.lastStep();
    
        bool     gate;
        uint32_t note;
    
        // Set tune and play colours
        if(_engine.clockRunning()) {
            gate = sequence.step(currentStep).gate();
            note = sequence.step(currentStep).note();
            ledDriver.setColourHSV(RGBLed::Code::Play, hueFromNote(note), gate ? 1.f : .1f, gate ? valueFromOctave(note) : .1f);
            if(_engine.selectedStep() >= 0) {
                note = sequence.step(_engine.selectedStep()).note();
            }
            ledDriver.setColourHSV(RGBLed::Code::Tune, hueFromNote(note), 1.f, 1.f);
        } else {
            ledDriver.setColourHSV(RGBLed::Code::Play, _pulse * 360.f, 1.f, 1.f);
    
            if(_engine.selectedStep() >= 0) {
                note = sequence.step(_engine.selectedStep()).note();
                ledDriver.setColourHSV(RGBLed::Code::Tune, hueFromNote(note), 1.f, 1.f);
            } else {
                ledDriver.setColourHSV(RGBLed::Code::Tune, 1.f, 0.f, 0.f); // off
            }
        }
    
        float pulseFrq = 2.f;
        // Set sequence button colours
        for(uint8_t step = firstStep; step <= lastStep; ++step) {
            gate = sequence.step(step).gate();
            note = sequence.step(step).note();
    
            if(gate || _keyState[step]) {
                float hue = hueFromNote(note);
                float saturation = 1.f;
                float value = valueFromOctave(note);
                if(step == currentStep) {
                    value += .2f;
                } else if(step == _engine.selectedStep() && !_engine.clockRunning()) {
                    // render triangle from pulse
                    value = 2 * fabsf(pulseFrq * _pulse - floorf(pulseFrq * _pulse + .5f)) * value;
                }
                CONSTRAIN(value, 0.f, 1.f);
                ledDriver.setColourHSV(fromKey(step), hue, saturation, value);
            } else if(step == currentStep) {
                ledDriver.setColourHSV(fromKey(step), 0.f, 0.f, .05f);
            }
        }
    }
        break;
    case Note:
    {
        ledDriver.setColourHSV(RGBLed::Code::Play, _pulse * 360.f, 1.f, 1.f);

        NoteSequence &sequence = static_cast<NoteTrackEngine*>(_engine.trackEngine())->sequence();
        int length = sequence.step(_engine.selectedStep()).length();
        for(uint8_t step = Key::Code::Step1; step <= Key::Code::Step8; ++step) {
            ledDriver.setColourHSV(fromKey(step), 0.f, 0.f, step <= length ? 1.f : 0.f);
        }
        ledDriver.setColourHSV(RGBLed::Code::Tune, 0.f, 0.f, length / 8.f);
    }
        break;
    case Sequence:
    {
        ledDriver.setColourHSV(RGBLed::Code::Play, _pulse * 360.f, 1.f, 1.f);
        /*
        Sequence_Pattern, -> 1..8
        Sequence_Swing, -> 50 ... 75
        Sequence_FirstStep, 1..8
        Sequence_LastStep, 1..8
        Sequence_RunMode, 1..6
        */
        ledDriver.setColourHSV(fromKey(Key::Code::Step1), colourTable8[_engine.trackEngine()->pattern()], 1.f, 1.f);
        ledDriver.setColourHSV(fromKey(Key::Code::Step2), 30.f, 1.f, 1.f);
        ledDriver.setColourHSV(fromKey(Key::Code::Step3), 120.f, 1.f, 1.f);
        ledDriver.setColourHSV(fromKey(Key::Code::Step4), 180.f, 1.f, 1.f);
        ledDriver.setColourHSV(fromKey(Key::Code::Step5), colourTable6[(uint8_t)static_cast<NoteTrackEngine*>(_engine.trackEngine())->sequence().runMode()], 1.f, 1.f);
        ledDriver.setColourHSV(fromKey(Key::Code::Step8), 0.f, 1.f, 1.f); // reset model

    }
        break;
    case Sequence_Pattern:
    {
        ledDriver.setColourHSV(RGBLed::Code::Play, _pulse * 360.f, 1.f, 1.f);
        uint8_t pattern = _engine.trackEngine()->pattern();
        for(int i = 0; i < 8; ++i) {
            ledDriver.setColourHSV(fromKey(i), colourTable8[i], 1.f, i == pattern ? 1.f : .05f);
        }
        ledDriver.setColourHSV(RGBLed::Code::Tune, colourTable8[pattern], 1.f, 1.f);
    }
        break;
    case Sequence_Swing:
    {
        ledDriver.setColourHSV(RGBLed::Code::Play, _pulse * 360.f, 1.f, 1.f);
        uint32_t swingId = roundf(remap(static_cast<float>(_engine.trackEngine()->swing() - 50), 25.f, 7.f));
        for(uint32_t i = 0; i < swingId; ++i) {
            ledDriver.setColourHSV(fromKey(i), 30.f, 1.f, 1.f);
        }
        ledDriver.setColourHSV(fromKey(swingId), 30.f, 1.f, 1.f);
        ledDriver.setColourHSV(RGBLed::Code::Tune, 30.f, 1.f, 1.f);
    }
        break;
    case Sequence_FirstStep:
    {
        ledDriver.setColourHSV(RGBLed::Code::Play, _pulse * 360.f, 1.f, 1.f);
        ledDriver.setColourHSV(fromKey(static_cast<NoteTrackEngine*>(_engine.trackEngine())->sequence().firstStep()), 120.f, 1.f, 1.f);
        ledDriver.setColourHSV(RGBLed::Code::Tune, 120.f, 1.f, 1.f);
    }
        break;
    case Sequence_LastStep:
    {
        ledDriver.setColourHSV(RGBLed::Code::Play, _pulse * 360.f, 1.f, 1.f);
        ledDriver.setColourHSV(fromKey(static_cast<NoteTrackEngine*>(_engine.trackEngine())->sequence().lastStep()), 180.f, 1.f, 1.f);
        ledDriver.setColourHSV(RGBLed::Code::Tune, 180.f, 1.f, 1.f);
    }
        break;
    case Sequence_RunMode:
    {
        ledDriver.setColourHSV(RGBLed::Code::Play, _pulse * 360.f, 1.f, 1.f);
        uint8_t runMode = (uint8_t)static_cast<NoteTrackEngine*>(_engine.trackEngine())->sequence().runMode();
        for(int i = 0; i < 6; ++i) {
            ledDriver.setColourHSV(fromKey(i), colourTable6[i], 1.f, i == runMode ? 1.f : .05f);
        }
        ledDriver.setColourHSV(RGBLed::Code::Tune, colourTable6[runMode], 1.f, 1.f);
    }
        break;
    default:
        break;
    }
}

void UiController::updateCV() {
    uint16_t value;
    for(size_t i = 0; i < CONFIG_NUM_POTS; ++i) {
        value = adc.channel(i);
        _cvReader.push(i, value); 
    }
}

void UiController::initializeFlash() {
    DBG("*********************************");
    DBG("** INITIALIZING MODEL IN FLASH **");
    DBG("*********************************");
    _model.init();
    _storage.write(_model);
    //uint32_t v[83];
    //_storage.read(v);
}

float UiController::hueFromNote(uint32_t note) {
    // Color: red         yellow       green         lightblue     darkblue      magenta       red
    // Hue  : 0° -------- 60° -------- 120° -------- 180° -------- 240° -------- 300° -------- 0°
    // Note : C           D            E             F#            G#            A#            C
    float delta = 4096.f / 61; // 5 Octaves * 12 semitones + 1 last C
    uint8_t k = floorf(note / delta);
    float hue = (k % 12) * 30; // 12 * 30 = 360, one loop per octave
    return hue;
}

float UiController::valueFromOctave(uint32_t note) {
    float delta = 4096.f / (61 / 12.f);
    uint8_t k = floorf(note / delta); // 0,...,5
    float value = .05f + 0.6f * (k / 5.f);
    return value;
}

RGBLed::Code UiController::fromKey(uint8_t keyCode) {
    RGBLed::Code code = RGBLed::Code::None;
    
    switch(keyCode) {
    case Key::Code::Step1: code = RGBLed::Code::Step1; break;
    case Key::Code::Step2: code = RGBLed::Code::Step2; break;
    case Key::Code::Step3: code = RGBLed::Code::Step3; break;
    case Key::Code::Step4: code = RGBLed::Code::Step4; break;
    case Key::Code::Step5: code = RGBLed::Code::Step5; break;
    case Key::Code::Step6: code = RGBLed::Code::Step6; break;
    case Key::Code::Step7: code = RGBLed::Code::Step7; break;
    case Key::Code::Step8: code = RGBLed::Code::Step8; break;
    case Key::Code::Play : code = RGBLed::Code::Play ; break;
    default: break;
    }

    return code;
}

void UiController::handleEvent(KeyEvent event) {
    //DBG("UiController::handleEvent type=%d, key=%d, count=%d", event.type(), event.key().code(), event.count());
    
    switch(_uiMode) {
    case Perform:
    {
        switch(event.type()) {
        case KeyEvent::KeyDown:
        {
            DBG("Engine::keyDown key=%d, count=%d", event.key().code(), event.count());
            //event.key().show();
        
            if(event.key().isStep()) {
                int selectedStep = event.key().code();
                _engine.setSelectedStep(selectedStep);
        
                NoteSequence &sequence = static_cast<NoteTrackEngine*>(_engine.trackEngine())->sequence();
                
                if(event.key().state(Key::Code::Shift)) {
                    if(event.count() == 1) {
                        sequence.step(selectedStep).toggleGate();
                    } else if(event.count() == 2) {
                        sequence.step(selectedStep).toggleGate(); // revert previous toggle :-)
                        _uiMode = Note;
                    }
                } else if(!_engine.clockRunning()) {
                    _engine.setGateOutputOverride(true);
                    _engine.setCvOutputOverride(true);
                    _engine.setGateOutput(true);
                    //_selectedStep = event.key().code();
                    _engine.setCvOutput(sequence.step(selectedStep).note());
                }
            } else if(event.key().isPlay()) {
                if(event.key().state(Key::Code::Shift)) {
                    // enter track edit mode
                    _uiMode = Sequence;
                } else {
                    _engine.togglePlay();
                    if(!_engine.clockRunning()) {
                        _storage.write(_model);
                    }
                }
            }
    
        }
            break;
        case KeyEvent::KeyUp:
        {
            if(event.key().isStep()) {
                //_engine.keyUp(event);
                DBG("Engine::keyUp   key=%d, count=%d, duration=%ld, isLong=%d", event.key().code(), event.count(), event.duration(), event.isLong());
                //event.key().show();
            
                if(event.key().isStep()) {
                    _engine.setGateOutput(false);
                    _engine.updateOverrides();
                    _engine.updatePeripherals();
                }
            
                if(event.key().none()) {
                    _engine.setGateOutputOverride(false);
                    _engine.setCvOutputOverride(false);
                }
            }
        }
            break;
        default:
            break;
        }
    }
        break;
    case Note:
    {
        if(event.type() == KeyEvent::KeyDown) {
            if(event.key().isStep()) {
                NoteSequence &sequence = static_cast<NoteTrackEngine*>(_engine.trackEngine())->sequence();
                int length = event.key().code();
                DBG("KeyEvent: Length=%d", length);
                sequence.step(_engine.selectedStep()).setLength(length);
            } else if(event.key().isPlay()) {
                _uiMode = Perform;
            }
        }
    }
        break;
    case Sequence:
    {
        /*
        _engine.trackEngine()->pattern();
        _engine.trackEngine()->changePattern();
        _engine.trackEngine()->swing();
        static_cast<NoteTrackEngine*>(_engine.trackEngine())->sequence();
        static_cast<NoteTrackEngine*>(_engine.trackEngine())->sequence().firstStep();
        static_cast<NoteTrackEngine*>(_engine.trackEngine())->sequence().lastStep();
        static_cast<NoteTrackEngine*>(_engine.trackEngine())->sequence().runMode();
        static_cast<NoteTrackEngine*>(_engine.trackEngine())->sequence().divisor();
        */
        
        if(event.type() == KeyEvent::KeyDown) {
            if(event.key().isStep()) {
                if(event.key().code() == Key::Code::Step1) {
                    _uiMode = Sequence_Pattern;
                } else if(event.key().code() == Key::Code::Step2) {
                    _uiMode = Sequence_Swing;
                } else if(event.key().code() == Key::Code::Step3) {
                    _uiMode = Sequence_FirstStep;
                } else if(event.key().code() == Key::Code::Step4) {
                    _uiMode = Sequence_LastStep;
                } else if(event.key().code() == Key::Code::Step5) {
                    _uiMode = Sequence_RunMode;
                }
            } else if(event.key().isPlay()) {
                _uiMode = Perform;
            }
        } else if(event.type() == KeyEvent::KeyUp) {
            if(event.key().isStep()) {
                if(event.key().code() == Key::Code::Step8 && event.isLong()) {
                    initializeFlash();
                }
            }
        }
    }
        break;
    case Sequence_Pattern:
    {
        if(event.type() == KeyEvent::KeyDown) {
            if(event.key().isStep()) {
                int newPattern = event.key().code();
                _engine.trackEngine()->setPattern(newPattern);
                _engine.trackEngine()->changePattern();
            } else if(event.key().isPlay()) {
                _uiMode = Sequence;
            }
        }
    }
        break;
    case Sequence_Swing:
    {
        if(event.type() == KeyEvent::KeyDown) {
            if(event.key().isStep()) {
                float swing = roundf(remap(event.key().code(), 7, 25) + 50);
                _model.project().setSwing(swing);
            } else if(event.key().isPlay()) {
                _uiMode = Sequence;
            }
        }
    }
        break;
    case Sequence_FirstStep:
    {
        if(event.type() == KeyEvent::KeyDown) {
            if(event.key().isStep()) {
                int firstStep = event.key().code();
                static_cast<NoteTrackEngine*>(_engine.trackEngine())->sequence().setFirstStep(firstStep);
            } else if(event.key().isPlay()) {
                _uiMode = Sequence;
            }
        }
    }
        break;
    case Sequence_LastStep:
    {
        if(event.type() == KeyEvent::KeyDown) {
            if(event.key().isStep()) {
                int lastStep = event.key().code();
                static_cast<NoteTrackEngine*>(_engine.trackEngine())->sequence().setLastStep(lastStep);
            } else if(event.key().isPlay()) {
                _uiMode = Sequence;
            }
        }
    }
        break;
    case Sequence_RunMode:
    {
        if(event.type() == KeyEvent::KeyDown) {
            if(event.key().isStep() && event.key().code() < 6) {
                Types::RunMode runMode = static_cast<Types::RunMode>(event.key().code());
                static_cast<NoteTrackEngine*>(_engine.trackEngine())->sequence().setRunMode(runMode);
            } else if(event.key().isPlay()) {
                _uiMode = Sequence;
            }
        }
    }
        break;
    default:
        break;
    }
}

void UiController::handleEvent(PotEvent event) {
    
    if(event.index() == 0) {

        switch(_uiMode) {
        case Perform:
        {
            // Pitch
            NoteSequence &sequence = static_cast<NoteTrackEngine*>(_engine.trackEngine())->sequence();
            //DBG("Set Pitch:%.2f", event.value());
            sequence.step(_engine.selectedStep()).setNote(event.value() * 0xFFF);
            
            if(_engine.gateOutputOverride()) {
                _engine.setCvOutput(sequence.step(_engine.selectedStep()).note());
            }
        }
            break;
        case Note:
        {
            int length = quantize(event.value() * 8, 8.f, 8.f);
            //DBG("PotEvent: Length=%d", length);
            NoteSequence &sequence = static_cast<NoteTrackEngine*>(_engine.trackEngine())->sequence();
            sequence.step(_engine.selectedStep()).setLength(length);
        }
            break;
        case Sequence:
        {
            
        }
            break;
        case Sequence_Pattern:
        {
            _engine.trackEngine()->setPattern(event.value() * 8);
        }
            break;
        case Sequence_Swing:
        {
            uint32_t swing = roundf(remap(static_cast<int>(event.value() * 8), 7, 25) + 50);
            _model.project().setSwing(swing);
        }
            break;
        case Sequence_FirstStep:
        {
                static_cast<NoteTrackEngine*>(_engine.trackEngine())->sequence().setFirstStep(event.value() * 8);
        }
            break;
        case Sequence_LastStep:
        {
                static_cast<NoteTrackEngine*>(_engine.trackEngine())->sequence().setLastStep(event.value() * 8);
        }
            break;
        case Sequence_RunMode:
        {
            Types::RunMode runMode = static_cast<Types::RunMode>(event.value() * 6.f);
            static_cast<NoteTrackEngine*>(_engine.trackEngine())->sequence().setRunMode(runMode);
        }
            break;
        default:
            break;
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