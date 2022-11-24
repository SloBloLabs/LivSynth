#include "LivSynthMain.h"
#include "main.h"
#include "swvPrint.h"
#include "System.h"
#include "ClockTimer.h"
#include "Clock.h"
#include "AdcInternal.h"
#include "DacInternal.h"
#include "Dio.h"
#include "ShiftRegister.h"
#include "ButtonMatrix.h"
#include "LEDDriver.h"
#include "Model.h"
#include "Engine.h"
#include "UiController.h"

#define CCMRAM_BSS __attribute__((section(".ccmram")))

static volatile float    _bpm;
static volatile float    _pitch;
static volatile uint32_t _dacValue;

static CCMRAM_BSS ClockTimer    clockTimer;
                  AdcInternal   adc;
       CCMRAM_BSS DacInternal   dac;
       CCMRAM_BSS Dio           dio;
static CCMRAM_BSS ShiftRegister shiftRegister;
       CCMRAM_BSS ButtonMatrix  buttonMatrix(shiftRegister);
                  LEDDriver     ledDriver;
static            Model         model;
static CCMRAM_BSS Engine        engine(model, clockTimer);
static CCMRAM_BSS UiController  uiController(model, engine);

void testMain() {
    System::init();
    //clockTimer.init();
    adc.init();
    //dac.init();
    shiftRegister.init();
    buttonMatrix.init();
    ledDriver.init();

    uint32_t curMillis
           , logMillis    = 0
           , updateMillis = 0;
    
    KeyState _keyState;
    _keyState.reset();

    while(true) {

        curMillis = System::ticks();

        if(curMillis - updateMillis > 79) {
            updateMillis = curMillis;

            buttonMatrix.process();

            uint16_t tune = adc.channel(0);
            uint16_t tempo = adc.channel(1);

            uint8_t numButtons = tune / 4095. * 9;
            float H = tempo / 4095. * 360;

            for(uint8_t button = 0; button < 8; ++button) {
                ledDriver.setColourHSV(button, H, 1.f, button < numButtons ? 1. : 0.);
            }
            
            static float hue = 0.f;
            hue += 10;
            if(hue >= 360.f) hue -= 360.f;

            ButtonMatrix::Event event;
            while(buttonMatrix.nextEvent(event)) {
                bool isDown = event.action() == ButtonMatrix::Event::KeyDown;

                _keyState[event.value()] = isDown;
                Key key(event.value(), _keyState);
                float value = isDown ? 0. : 1.;
                if(key.isStep()) {
                    ledDriver.setColourHSV(event.value(), hue, 1.f, value);
                } else if(key.isPlay()) {
                    ledDriver.setColourHSV(8, hue, 1.f, value);
                } else if(key.isShift()) {
                    ledDriver.setColourHSV(9, hue, 1.f, value);
                }
            }

            ledDriver.process();
            
            LL_GPIO_TogglePin(CLOCK_OUT_GPIO_Port, CLOCK_OUT_Pin);
        }
        
        // render debug log output
        if(curMillis - logMillis > 999) {
            logMillis = curMillis;
            DBG("Tune_ADC=%d, Tempo_ADC=%d", adc.channel(0), adc.channel(1));
            LL_GPIO_TogglePin(RESET_OUT_GPIO_Port, RESET_OUT_Pin);
        }
    }
}

void appMain() {
    System::init();
    clockTimer.init();
    adc.init();
    dac.init();
    shiftRegister.init();
    buttonMatrix.init();
    ledDriver.init();
    engine.init();
    uiController.init();

    stopSequencer();

    _bpm = 120.;

    startSequencer();

    uint32_t curMillis
           , logMillis    = 0
           , updateMillis = 0
           , engineMillis = 0;
    
    bool debug = false;
    
    while(true) {

        curMillis = System::ticks();

        if(curMillis - engineMillis > 1) {
            engineMillis = curMillis;
            uiController.handleControls();
            bool updated = engine.update();
            if(updated) {
                uiController.renderSequence();
                ledDriver.process();
                //DBG("Update LEDs");
            }
        }
        
        // update sequencer input and state
        if(curMillis - updateMillis > 79) {
            updateMillis = curMillis;

            /*static uint8_t curLed = 0, lastLed = 0;
            static float hue = 0.;

            for(uint8_t led = 0; led < 8; ++led) {
                ledDriver.setColourHSV(led, hue, 1., 1.);
            }
            hue += 10;
            if(hue >= 360.) hue -= 360.;

            ledDriver.setSingleLED(lastLed, 0x0);
            ledDriver.setSingleLED(curLed, 0xFFF);
            lastLed = curLed;
            if(!(++curLed % 15)) curLed++;
            if(curLed > 24) curLed = 0;*/

            //ledDriver.process();

            buttonMatrix.process();
            setTempo();
            setPitch();
        }
        
        // render debug log output
        if(debug && curMillis - logMillis > 999) {
            logMillis = curMillis;
            DBG("ADC0=%d, ADC1=%d, bpm=%.2f, pitch=%.2f, buttons=0x%02X", adc.channel(0), adc.channel(1), _bpm, _pitch, shiftRegister.read());
        }
    }
}

// glue code
void appTick(void) {
    System::tick();
}

void appClockTimer() {
    clockTimer.notifyTimerUpdate();
}

void appLEDTxComplete() {
    ledDriver.notifyTxComplete();
}

void appLEDTxError() {
    ledDriver.notifyTxError();
}

void appADCCompleteRequest() {
    // only called if interrupt is enabled
    // see AdcInternal.cpp -> LL_DMA_EnableIT_TC(DMA2, LL_DMA_STREAM_0)
    uiController.updateCV();
}

float adc2bpm(uint16_t adcValue) {
    /*
    0 - 4095 ^= 20 - 300 bpm
    0    -> 20
    4095 -> 300
    f(x) = mx + b = dy/dx * x + b = (300 - 20) / 4095 * x + b
    f(0) = 280/4095 * 0 + b = 20
    f(x) = 280/4096 * x + 20
    */
   return 280.f/4096 * adcValue + 20.;
}

float adc2Volt(uint16_t adcValue) {
    /*
    we want 5 octaves
    0 - 4096 ^= 0 - 5 Volt
    f(x) = 5/4096 * x
    */
    return 5.f/4096 * adcValue;
}

void startSequencer() {
    setTempo();
    engine.clockStart();
    DBG("Sequencer started.");
}

void stopSequencer() {
    engine.clockStop();
    DBG("Sequencer stopped.");
}

void setTempo() {
    _bpm = adc2bpm(adc.channel(0));
    //DBG("Tempo: %.2f", bpm);
}

void setPitch() {
    _pitch = adc2Volt(adc.channel(1));
    //DBG("Pitch: %.2f", _pitch);
}