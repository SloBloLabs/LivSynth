#include "UiController.h"
#include "System.h"
#include "NoteTrackEngine.h"
#include "swvPrint.h"

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
}

/*void UiController::update() {
    handleKeys();

    _leds.clear();
    renderSequence();
}*/

void UiController::handleControls() {
    ButtonMatrix::Event event;
    while(buttonMatrix.nextEvent(event)) {
        bool isDown = event.action() == ButtonMatrix::Event::KeyDown;
        _keyState[event.value()] = isDown;
        Key key(event.value(), _keyState);
        if(isDown) {
            KeyEvent keyPressEvent = _keyPressEventTracker.process(key);
            //DBG("KeyPressEvent type=%d, key=%d, count=%d", keyPressEvent.type(), keyPressEvent.key().code(), keyPressEvent.count());
            handleEvent(keyPressEvent);
        } else {
            KeyEvent keyEvent(KeyEvent::KeyUp, key, 1);
            handleEvent(keyEvent);
        }
    }

    float cvValue;
    for(size_t i = 0; i < CONFIG_NUM_POTS; ++i) {
        cvValue = _cvReader.getCV(i);
        //DBG("%d ov:%.2f, nv:%.2f, diff:%.2f", i, _cvValue[i], cvValue, cvValue - _cvValue[i]);
        if(std::abs(cvValue - _cvValue[i]) > 0.02) {
            // trigger event
            //DBG("cv value updated: index=%d, value=%.2f", i, cvValue);
            _cvValue[i] = cvValue;
            PotEvent potEvent(i, cvValue);
            handleEvent(potEvent);
        }
    }
}

void UiController::renderSequence() {
    ledDriver.clear();
    uint8_t pattern = _engine.trackEngine()->pattern();
    uint8_t currentStep = reinterpret_cast<NoteTrackEngine*>(_engine.trackEngine())->currentStep();
    NoteSequence &sequence = _model.project().noteSequence(pattern);
    uint8_t firstStep = sequence.firstStep();
    uint8_t lastStep = sequence.lastStep();
    bool gate;
    uint32_t note;
    for(uint8_t step = firstStep; step <= lastStep; ++step) {
        gate = sequence.step(step).gate();
        note = sequence.step(step).note();

        if(gate || _keyState[step]) {
            // 68 per half step note
            // 5 octaves -> 5 * 12 * 68 = 4080
            // 4095 max
            // 360° ^= 12 * 68 = 816
            // Color: red         yellow       green         lightblue     darkblue      magenta       red
            // Hue  : 0° -------- 60° -------- 120° -------- 180° -------- 240° -------- 300° -------- 0°
            // Note : C           D            E             F#            G#            A#            C
            // 12bit: 0           136          272           408           544           680           816
            // 12bit value = octave [0-4] * 816 + hue * 816 / 360
            ledDriver.setColourHSV(step, hueFromNote(note), 1.f, step == currentStep ? 1.f : .1f);
        } else if(step == currentStep) {
            ledDriver.setColourHSV(step, 0.f, 0.f, .05f);
        }
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
    return ((note % 816) * 360.f) / 816;
}

void UiController::handleEvent(KeyEvent event) {
    //DBG("KeyPressEvent type=%d, key=%d, count=%d", event.type(), event.key().code(), event.count());
    switch(event.type()) {
    case KeyEvent::KeyDown:
        _engine.keyDown(event);
        break;
    case KeyEvent::KeyUp:
        _engine.keyUp(event);
        break;
    default:
        break;
    }
}

void UiController::handleEvent(PotEvent event) {
    if(event.index() == 0) {
        // Pitch
        _engine.setCV(event);
    } else if(event.index() == 1) {
        // Tempo
    }
}