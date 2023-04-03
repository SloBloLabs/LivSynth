#include "Engine.h"
#include "AdcInternal.h"
#include "DacInternal.h"
#include "ButtonMatrix.h"
#include "LEDDriver.h"
#include "RGBLed.h"
#include "Event.h"
#include "KeyPressEventTracker.h"
#include "CVReader.h"
#include "FlashStorage.h"

#define red         0.f
#define orange     30.f
#define yellow     60.f
#define green     120.f
#define lightblue 180.f
#define blue      240.f
#define violet    270.f
#define magenta   300.f

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
    
    void initializeFlash();
    float hueFromNote(uint32_t note);
    float valueFromOctave(uint32_t note);
    RGBLed::Code fromKey(uint8_t keyCode);
    void handleEvent(KeyEvent event);
    void handleEvent(PotEvent event);

    float colourTable6[6] = {red,         yellow, green, lightblue, blue,         magenta};
    float colourTable8[8] = {red, orange, yellow, green, lightblue, blue, violet, magenta};

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

    FlashStorage<3> _storage;
};