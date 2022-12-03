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

#define RUN_TEST 0

#define CCMRAM_BSS __attribute__((section(".ccmram")))

static volatile float    _bpm;
static volatile float    _pitch;

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

void appMain() {

    if(RUN_TEST) testMain();

    System::init();
    clockTimer.init();
    dio.init();
    adc.init();
    dac.init();
    shiftRegister.init();
    buttonMatrix.init();
    ledDriver.init();
    engine.init();
    uiController.init();

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
                uiController.renderUI();
                ledDriver.process();
                //DBG("Update LEDs");
            }
        }
        
        // update sequencer input and state
        if(curMillis - updateMillis > 49) {
            updateMillis = curMillis;

            buttonMatrix.process();

            if(!engine.clockRunning()) {
                uiController.renderUI();
                ledDriver.process();
            }
        }
        
        // render debug log output
        if(debug && curMillis - logMillis > 999) {
            logMillis = curMillis;
            DBG("ADC0=%d, ADC1=%d, bpm=%.2f, pitch=%.2f, buttons=0x%02X", adc.channel(0), adc.channel(1), _bpm, _pitch, shiftRegister.read());
        }
    }
}

// NVIC glue code
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
    uiController.updateCV();
}

/*#######################################
   A fun test application to check
     MCU peripherals and devices
#######################################*/
void testMain() {
    System::init();
    //clockTimer.init();
    dio.init();
    adc.init();
    dac.init();
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

            dac.setValue(tune);

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
                float value = isDown ? 1. : 0.;
                if(key.isStep()) {
                    ledDriver.setColourHSV(event.value(), hue, 1.f, value);
                } else if(key.isPlay()) {
                    ledDriver.setColourHSV(8, hue, 1.f, value);
                } else if(key.isShift()) {
                    ledDriver.setColourHSV(9, hue, 1.f, value);
                    dio.setGate(isDown);
                }
            }

            ledDriver.process();
            dio.update();
            dac.update();

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