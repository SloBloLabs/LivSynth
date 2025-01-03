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
#include <cmath>
#include "MidiHandler.h"

#define RUN_TEST 0

#define CCMRAM_BSS __attribute__((section(".ccmram")))

static CCMRAM_BSS ClockTimer    clockTimer;
                  AdcInternal   adc;
       CCMRAM_BSS DacInternal   dac;
       CCMRAM_BSS Dio           dio;
static CCMRAM_BSS ShiftRegister shiftRegister;
       CCMRAM_BSS ButtonMatrix  buttonMatrix(shiftRegister);
                  LEDDriver     ledDriver;
static            Model         model;
static CCMRAM_BSS MidiHandler   midiHandler;
static CCMRAM_BSS Engine        engine(model, clockTimer, midiHandler);
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
    midiHandler.init();
    engine.init();
    uiController.init();

    uint32_t curMillis
           , logMillis    = 0
           , engineMillis = 0;
    
    bool debug = false;
    
    bool updated = false;

    while(true) {

        curMillis = System::ticks();

        if(curMillis - engineMillis > 1) {
            //LL_GPIO_SetOutputPin(DBG1_GPIO_Port, DBG1_Pin);
            engineMillis = curMillis;
            updated = engine.update() || updated;

            buttonMatrix.process();
            uiController.handleControls(curMillis);

            if(updated || !engine.clockRunning()) {
                uiController.renderUI();
                ledDriver.process();
                updated = false;
            }
        	//LL_GPIO_ResetOutputPin(DBG1_GPIO_Port, DBG1_Pin);
        }
        
        // render debug log output
        if(debug && curMillis - logMillis > 999) {
            logMillis = curMillis;
            UDBG("Tempo = %.2f, RunState = %d\n", engine.tempo(), engine.runState());
            //DBG("ADC0=%d, ADC1=%d, bpm=%.2f, pitch=%.2f, buttons=0x%02X", adc.channel(0), adc.channel(1), _bpm, _pitch, shiftRegister.read());
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

extern "C" {

void enqueueIncomingMidi(uint8_t *data) {
    //LL_GPIO_SetOutputPin(DBG2_GPIO_Port, DBG2_Pin);
    MidiUSBMessage umsg(data);
    MidiMessage msg;
    umsg.getMidiMessage(msg);
    midiHandler.enqueueIncoming(msg);
    //LL_GPIO_ResetOutputPin(DBG2_GPIO_Port, DBG2_Pin);
}

void midiTrxSentCallback() {
    midiHandler.setBusy(false);
}

}

/*#######################################
   A fun test application to check
     MCU peripherals and devices
#######################################*/
void testMain() {
    System::init();
    clockTimer.init();
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

    uint8_t demoStyle = 0;

    while(true) {

        curMillis = System::ticks();

        if(curMillis - updateMillis > 79) {
            updateMillis = curMillis;

            buttonMatrix.process();

            uint16_t tune = adc.channel(0);
            uint16_t tempo = adc.channel(1);

            dac.setValue(tune);
            
            if(demoStyle == 0) {
                float delta = 4095.0 / 10;
                uint8_t k = floorf(tune / delta + .5f);
                
                float H = tempo / 4095. * 360;
    
                for(uint8_t button = 0; button < 10; ++button) {
                    ledDriver.setColourHSV(button, H, 1.f, button < k ? 1. : 0.);
                }
                
                static float hue = 0.f;
                hue += 10;
                if(hue >= 360.f) hue -= 360.f;
    
                ButtonMatrix::Event event;
                while(buttonMatrix.nextEvent(event)) {
                    bool isDown = event.action() == ButtonMatrix::Event::KeyDown;
    
                    DBG("action=%d, value=%d", event.action(), event.value());
                    _keyState[event.value()] = isDown;
                    Key key(event.value(), _keyState);
                    float value = isDown ? 1. : 0.;
                    if(key.isStep()) {
                        ledDriver.setColourHSV(event.value() + 1, hue, 1.f, value);
                    } else if(key.isPlay()) {
                        ledDriver.setColourHSV(RGBLed::Code::Play, hue, 1.f, value);
                        dio.setGate(isDown);
                    } else if(key.isShift()) {
                        ledDriver.setColourHSV(RGBLed::Code::Tune, hue, 1.f, value);
                        if(!isDown)
                            demoStyle = 1;
                    }
                }
            } else if(demoStyle == 1) {
                float saturation = tune / 4095.f;
                float value      = tempo / 4095.f;

                for(uint8_t led = 0; led < 11; ++led) {
                    ledDriver.setColourHSV(led, 0.f, saturation, value);
                }

                ButtonMatrix::Event event;
                while(buttonMatrix.nextEvent(event)) {
                    bool isDown = event.action() == ButtonMatrix::Event::KeyDown;
    
                    DBG("action=%d, value=%d", event.action(), event.value());
                    _keyState[event.value()] = isDown;
                    Key key(event.value(), _keyState);
                    if(key.isStep()) {
                        //ledDriver.setColourHSV(event.value() + 1, 0.f, saturation, value);
                    } else if(key.isPlay()) {
                        //ledDriver.setColourHSV(RGBLed::Code::Play, 0.f, saturation, value);
                        dio.setGate(isDown);
                    } else if(key.isShift()) {
                        ledDriver.setColourHSV(RGBLed::Code::Tune, 0.f, saturation, value);
                        if(!isDown)
                            demoStyle = 0;
                    }
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
