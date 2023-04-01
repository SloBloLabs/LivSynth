#include "Engine.h"
#include "AdcInternal.h"
#include "DacInternal.h"
#include "ButtonMatrix.h"
#include "LEDDriver.h"
#include "RGBLed.h"
#include "Event.h"
#include "KeyPressEventTracker.h"
#include "CVReader.h"

typedef std::array<uint32_t, CONFIG_NUM_BUTTONS> KeyPressDurationTracker;
class UiController {
public:
    UiController(Model &model, Engine &engine) :
        _model(model),
        _engine(engine)
    {}

    enum UiMode {
        Perform,
        Note,
        Sequence,
        Sequence_Pattern,
        Sequence_Swing,
        Sequence_FirstStep,
        Sequence_LastStep,
        Sequence_RunMode,
        UiMode_Last
    };

    void init();
    void handleControls(uint32_t time);
    void renderUI();
    void updateCV();

private:
    
    float hueFromNote(uint32_t note);
    float valueFromOctave(uint32_t note);
    RGBLed::Code fromKey(uint8_t keyCode);
    void handleEvent(KeyEvent event);
    void handleEvent(PotEvent event);

    float colourTable6[6] = {0.f, 60.f, 120.f, 180.f, 240.f, 300.f};
    float colourTable8[8] = {0.f, 30.f, 60.f, 120.f, 180.f, 240.f, 270.f, 300.f};

    Model &_model;
    Engine &_engine;

    KeyState _keyState;
    KeyPressEventTracker _keyPressEventTracker;
    KeyPressDurationTracker _keyPressDurationTracker;

    uint32_t _lastControllerUpdateTicks;

    CVReader _cvReader;
    float _cvValue[CONFIG_NUM_POTS];
    float _pulse;

    UiMode _uiMode;
};