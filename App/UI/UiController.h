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
#include "KnownDivisor.h"

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
        Sequence_Divisor,
        Clock_OutputDivisor,
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

    float _colourTable6[6] = {red,         yellow, green, lightblue, blue,         magenta};
    float _colourTable8[8] = {red, orange, yellow, green, lightblue, blue, violet, magenta};

    KnownDivisor _divisorSelection[8] = {
        knownDivisors[3],  // 1/32
        knownDivisors[4],  // 1/16T
        knownDivisors[5],  // 1/32.
        knownDivisors[6],  // 1/16
        knownDivisors[7],  // 1/8T
        knownDivisors[8],  // 1/4T
        knownDivisors[9],  // 1/8.
        knownDivisors[10]  // 1/4
    };

    uint8_t findDivisorIndex(uint8_t divisor);

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

    FlashStorage<4> _storage;
};
