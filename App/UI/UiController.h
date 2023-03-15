#include "Engine.h"
#include "AdcInternal.h"
#include "DacInternal.h"
#include "ButtonMatrix.h"
#include "LEDDriver.h"
#include "RGBLed.h"
#include "Event.h"
#include "KeyPressEventTracker.h"
#include "CVReader.h"

class UiController {
public:
    UiController(Model &model, Engine &engine) :
        _model(model),
        _engine(engine)
    {}

    void init();
    void handleControls();
    void renderUI();
    void updateCV();

private:
    
    float hueFromNote(uint32_t note);
    float valueFromOctave(uint32_t note);
    RGBLed::Code fromKey(uint8_t keyCode);
    void handleEvent(KeyEvent event);
    void handleEvent(PotEvent event);

    Model &_model;
    Engine &_engine;

    KeyState _keyState;
    KeyPressEventTracker _keyPressEventTracker;

    uint32_t _lastControllerUpdateTicks;

    CVReader _cvReader;
    float _cvValue[CONFIG_NUM_POTS];
    float _pulse;
};