#include "UiController.h"
#include "System.h"
#include "NoteTrackEngine.h"
#include "swvPrint.h"
#include "Utils.h"
#include <cmath>

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
    case Sequence:
    {
        for(uint8_t step = Key::Code::Step1; step <= Key::Code::Play; ++step) {
            ledDriver.setColourHSV(fromKey(step), 120.f, 1.f, 1.f);
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
        ledDriver.setColourHSV(RGBLed::Code::Tune, 0.f, 0.f, length/ 8.f);
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
            //_engine.keyDown(event);
    
            DBG("Engine::keyDown key=%d, count=%d", event.key().code(), event.count());
            //event.key().show();
        
        
            if(event.key().isStep()) {
                int selectedStep = event.key().code();
                _engine.setSelectedStep(selectedStep);
        
                NoteSequence &sequence = static_cast<NoteTrackEngine*>(_engine.trackEngine())->sequence();
                
                if(event.key().state(Key::Code::Shift)) {
                    if(event.count() > 1) {
                        sequence.step(event.key().code()).toggleGate();
                    }
                } else {
                    _engine.setGateOutputOverride(true);
                    _engine.setCvOutputOverride(true);
                    _engine.setGateOutput(true);
                    //_selectedStep = event.key().code();
                    _engine.setCvOutput(sequence.step(selectedStep).note());
                }
            } else if(event.key().isPlay() && !event.key().state(Key::Code::Shift)) {
                _engine.togglePlay();
            }
    
        }
            break;
        case KeyEvent::KeyUp:
        {
            if(event.key().isPlay() && event.key().state(Key::Code::Shift)) {
                // enter track edit mode
                _uiMode = Sequence;
            } else if(event.key().isStep()) {
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
                
                if(event.isLong()) {
                    _uiMode = Note;
                }
            } else {
            }
        }
            break;
        default:
            break;
        }
    }
        break;
    case Sequence:
    {
        switch(event.type()) {
        case KeyEvent::KeyDown:
        {
        }
            break;
        case KeyEvent::KeyUp:
        {
            if(event.key().isPlay()) {
                _uiMode = Perform;
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
        switch(event.type()) {
        case KeyEvent::KeyDown:
        {
            if(event.key().isStep()) {
                NoteSequence &sequence = static_cast<NoteTrackEngine*>(_engine.trackEngine())->sequence();
                int length = event.key().code();
                DBG("KeyEvent: Length=%d", length);
                sequence.step(_engine.selectedStep()).setLength(length);
            }
        }
            break;
        case KeyEvent::KeyUp:
        {
            if(event.key().isPlay()) {
                _uiMode = Perform;
            }
        }
            break;
        default:
            break;
    
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
            if(_engine.selectedStep() != -1) {
                NoteSequence &sequence = static_cast<NoteTrackEngine*>(_engine.trackEngine())->sequence();
                //DBG("Set Pitch:%.2f", event.value());
                sequence.step(_engine.selectedStep()).setNote(event.value() * 0xFFF);
                
                if(_engine.gateOutputOverride()) {
                    _engine.setCvOutput(sequence.step(_engine.selectedStep()).note());
                }
            }
        }
            break;
        case Sequence:
        {
            
        }
            break;
        case Note:
        {
            int length = quantize(event.value() * 8, 8.f, 8.f);
            DBG("PotEvent: Length=%d", length);

            NoteSequence &sequence = static_cast<NoteTrackEngine*>(_engine.trackEngine())->sequence();
            sequence.step(_engine.selectedStep()).setLength(length);
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