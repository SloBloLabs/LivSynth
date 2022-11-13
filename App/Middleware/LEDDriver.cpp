#include "LEDDriver.h"
#include "System.h"
#include "math.h"
#include "swvPrint.h"
#include "Utils.h"

#define USE_DMA 1

// http://stefanfrings.de/stm32/stm32f1.html#i2c
// https://community.st.com/s/question/0D50X00009bLPuwSAG/busy-bus-after-i2c-reading

void LEDDriver::init() {
    //LL_I2C_Enable(LED_I2C); // -> already called by LL_I2C_Init

    if(USE_DMA) {
        //LL_I2C_EnableIT_EVT(LED_I2C);
        //LL_I2C_EnableIT_ERR(LED_I2C);

        LL_DMA_DisableStream(DMA1, LL_DMA_STREAM_7);
    }

    _chipAddressArray = {1, 2};

    _ledTypeArray[0] = {
        COMMON_ANODE,
        COMMON_ANODE,
        COMMON_ANODE,
        COMMON_ANODE,
        COMMON_ANODE,
        COMMON_ANODE,
        COMMON_ANODE,
        COMMON_ANODE,
        COMMON_ANODE,
        COMMON_ANODE,
        COMMON_ANODE,
        COMMON_ANODE,
        COMMON_CATHODE,
        COMMON_CATHODE,
        COMMON_CATHODE,
        EMPTY};
    
    _ledTypeArray[1] = {
        COMMON_CATHODE,
        COMMON_CATHODE,
        COMMON_CATHODE,
        COMMON_CATHODE,
        COMMON_CATHODE,
        COMMON_CATHODE,
        COMMON_CATHODE,
        COMMON_CATHODE,
        COMMON_CATHODE,
        EMPTY,
        EMPTY,
        EMPTY,
        EMPTY,
        EMPTY,
        EMPTY,
        EMPTY};
    
    _transmissionBusy = 0;
    _curChip = 0;

    for(uint32_t chip = 0; chip < NUM_PWM_LED_CHIPS; ++chip) {
        resetChip(chip);
    }
    
    clear();

    // Tie to ground! Disabling brings common cathode LEDs full on.
    ledEnable();
}

void LEDDriver::process() {
    if(_transmissionBusy) {
        return;
    }

    //_pwmLeds[0][0][LED_ON] = 0xFFF; // LED 1 RED
    //_pwmLeds[0][0][LED_OFF] = 0x1000; // LED 1 RED
    //_pwmLeds[0][1][LED_OFF] = 0x0FFF; // LED 1 GREEN
    //_pwmLeds[0][2][LED_OFF] = 0x0FFF; // LED 1 BLUE
    //_pwmLeds[0][3][LED_OFF] = 0x0FFF; // LED 2 RED
    //_pwmLeds[0][4][LED_OFF] = 0x0FFF; // LED 2 GREEN
    //_pwmLeds[0][5][LED_OFF] = 0x0FFF; // LED 2 BLUE
    //_pwmLeds[0][6][LED_OFF] = 0x0FFF; // LED 3 RED
    //_pwmLeds[0][7][LED_OFF] = 0x0FFF; // LED 3 GREEN
    //_pwmLeds[0][8][LED_OFF] = 0x0FFF; // LED 3 BLUE
    //_pwmLeds[0][9][LED_OFF] = 0x0FFF; // LED 4 RED
    //_pwmLeds[0][10][LED_OFF] = 0x0FFF; // LED 4 GREEN
    //_pwmLeds[0][11][LED_OFF] = 0x0FFF; // LED 4 BLUE
    //_pwmLeds[0][12][LED_ON] = 0x1000; // LED 5 RED
    //_pwmLeds[0][12][LED_OFF] = 0xFFF; // LED 5 RED
    //_pwmLeds[0][13][LED_ON] = 0x1FFF; // LED 5 GREEN
    //_pwmLeds[0][13][LED_OFF] = 0x0; // LED 5 GREEN
    //_pwmLeds[0][14][LED_ON] = 0x1FFF; // LED 5 BLUE
    //_pwmLeds[0][14][LED_OFF] = 0x0; // LED 5 BLUE
    //_pwmLeds[0][15][LED_OFF] = 0x0FFF; // EMPTY
    //_pwmLeds[1][0][LED_ON] = 0x1FFF; // LED 6 RED
    //_pwmLeds[1][0][LED_OFF] = 0x0; // LED 6 RED
    //_pwmLeds[1][1][LED_ON] = 0x1FFF; // LED 6 GREEN
    //_pwmLeds[1][1][LED_OFF] = 0x0; // LED 6 GREEN
    //_pwmLeds[1][2][LED_ON] = 0x1FFF; // LED 6 BLUE
    //_pwmLeds[1][2][LED_OFF] = 0x0; // LED 6 BLUE
    //_pwmLeds[1][3][LED_ON] = 0x1FFF; // LED 7 RED
    //_pwmLeds[1][3][LED_OFF] = 0x0; // LED 7 RED
    //_pwmLeds[1][4][LED_ON] = 0x1FFF; // LED 7 GREEN
    //_pwmLeds[1][4][LED_OFF] = 0x0; // LED 7 GREEN
    //_pwmLeds[1][5][LED_ON] = 0x1FFF; // LED 7 BLUE
    //_pwmLeds[1][5][LED_OFF] = 0x0; // LED 7 BLUE
    //_pwmLeds[1][6][LED_ON] = 0x1FFF; // LED 8 RED
    //_pwmLeds[1][6][LED_OFF] = 0x0; // LED 8 RED
    //_pwmLeds[1][7][LED_ON] = 0x1FFF; // LED 8 GREEN
    //_pwmLeds[1][7][LED_OFF] = 0x0; // LED 8 GREEN
    //_pwmLeds[1][8][LED_ON] = 0x1FFF; // LED 8 BLUE
    //_pwmLeds[1][8][LED_OFF] = 0x0; // LED 8 BLUE

    if(USE_DMA) {
        _curChip = 0;
        _transmissionBusy = 1;
        writeRegistersDMA(_curChip, PCA9685_LED0, reinterpret_cast<uint8_t*>(_pwmLeds[_curChip]), 15 << 2);
    } else {
        ErrorStatus status;
        for(_curChip = 0; _curChip < NUM_PWM_LED_CHIPS; ++_curChip) {
            status = writeRegisters(_curChip, PCA9685_LED0, reinterpret_cast<uint8_t*>(_pwmLeds[_curChip]), 15 << 2);
            if(status == ERROR) {
                DBG("error writing to register");
            }
        }
    }
}

void LEDDriver::notifyTxComplete() {
    // DBG("LED Tx Complete!");
    stopTransfer();
    LL_DMA_DisableStream(DMA1, LL_DMA_STREAM_7);
    if(++_curChip < NUM_PWM_LED_CHIPS) {
        writeRegistersDMA(_curChip, PCA9685_LED0, reinterpret_cast<uint8_t*>(_pwmLeds[_curChip]), 15 << 2);
    } else {
        _transmissionBusy = 0;
    }
}

void LEDDriver::notifyTxError() {
    DBG("LED Tx Error!");
    NVIC_DisableIRQ(DMA1_Stream7_IRQn);
}

void LEDDriver::clear() {
    if(_transmissionBusy) {
        return;
    }

    for(uint32_t chip = 0; chip < NUM_PWM_LED_CHIPS; ++chip) {
        for(uint32_t led = 0; led < NUM_LEDS_PER_CHIP; ++led) {
            _pwmLeds[chip][led][LED_ON] = _ledTypeArray[chip][led] == COMMON_CATHODE ? 0x1000 : 0;
            _pwmLeds[chip][led][LED_OFF] = 0;
        }
    }
}

//|---------------|---------------------------------|------------------------------------------|
//|               |      RGB LED / CommonCathode    |     TC002-N11AS2XT-RGB / CommonAnode     |
//|---------------|---------------------------------|------------------------------------------|
//|               | LED_ON                | LED_OFF | LED_ON                         | LED_OFF |
//|---------------|-----------------------|---------|--------------------------------|---------|
//| LED full off  | 0x1XXX                | 0xX     | 0x0                            | 0x0     |
//| LED full on   | 0x0                   | 0x0     | 0xX                            | 0x1XXX  | <- not implemented
//| LED 1/4095    | 0x0                   | 0xFFF   | 0x0                            | 0x1     |
//| LED 4094/4095 | 0x0                   | 0x1     | 0x0                            | 0xFFF   |

void LEDDriver::setSingleLED(uint8_t led, uint16_t brightness) {
    if(_transmissionBusy) {
        return;
    }
    if(led < NUM_PWM_LED_CHIPS * NUM_LEDS_PER_CHIP) {
        uint32_t chipNumber = led / NUM_LEDS_PER_CHIP;
        led -= chipNumber * NUM_LEDS_PER_CHIP;
        if(_ledTypeArray[chipNumber][led] == COMMON_CATHODE) {
            if(brightness == 0) {
                _pwmLeds[chipNumber][led][LED_ON]= 0x1000;
            } else {
                _pwmLeds[chipNumber][led][LED_ON]= 0x0;
                _pwmLeds[chipNumber][led][LED_OFF]= 0x1000 - brightness;
            }
        } else {
            _pwmLeds[chipNumber][led][LED_OFF]= brightness;
        }
    }
}

void LEDDriver::setColourRGB(uint8_t colourLED, float r, float g, float b) {
    uint8_t startLed = colourLED * 3 + floor(colourLED / 5);
    setSingleLED(startLed++, MAX_R_VALUE * r);
    setSingleLED(startLed++, MAX_G_VALUE * g);
    setSingleLED(startLed  , MAX_B_VALUE * b);
}

void LEDDriver::setColourHSV(uint8_t colourLED, float H, float S, float V) {
    float r = 0., g = 0., b = 0.;
    HSVtoRGB(H, S, V, r, g, b);
    //DBG("H=%.2f, r=%.2f, g=%.2f, b=%.2f", H, r, g, b);
    setColourRGB(colourLED, r, g, b);
}

void LEDDriver::resetChip(uint32_t chipNumber) {
    // clear sleep mode
    ErrorStatus status = writeSingleRegister(chipNumber, PCA9685_MODE1, 0b00000000);
    if(status == ERROR) {
        DBG("error writing to register");
    }
    LL_mDelay(1);

    // start reset mode
    status = writeSingleRegister(chipNumber, PCA9685_MODE1, 0b10000000);
    if(status == ERROR) {
        DBG("error writing to register");
    }
    LL_mDelay(1);

    // enable auto increment
    status = writeSingleRegister(chipNumber, PCA9685_MODE1, 0b00100000);
    if(status == ERROR) {
        DBG("error writing to register");
    }
    LL_mDelay(1);

    // INVERT=1, OUTDRV=0, OUTNE=01, Totem Pole to drive common cathode LEDs
    status = writeSingleRegister(chipNumber, PCA9685_MODE2, 0b00010101);
    if(status == ERROR) {
        DBG("error writing to register");
    }
    LL_mDelay(1);
}

ErrorStatus LEDDriver::writeSingleRegister(uint8_t chipNumber, uint8_t registerAddress, uint8_t registerValue) {
    return writeRegisters(chipNumber, registerAddress, &registerValue, 1);
}

ErrorStatus LEDDriver::writeRegisters(uint8_t chipNumber, uint8_t startRegister, uint8_t* registerValues, uint32_t count) {
    ErrorStatus ret = SUCCESS, status;
    
    status = startTransfer();
    if(status == SUCCESS) {
        status = sendAddress(_chipAddressArray[chipNumber]);
        if(status == ERROR) {
            ret = status;
        }
    
        status = sendData(startRegister);
        if(status == ERROR) {
            ret = status;
        }

        for(uint32_t i = 0; i < count; ++i) {
            status = sendData(registerValues[i]);
            if(status == ERROR) {
                ret = status;
            }
        }
    } else {
        ret = status;
    }

    stopTransfer();

    return ret;
}

ErrorStatus LEDDriver::writeRegistersDMA(uint8_t chipNumber, uint8_t startRegister, uint8_t* registerValues, uint32_t count) {
    ErrorStatus ret = SUCCESS, status;

    status = startTransfer();
    if(status == SUCCESS) {
        status = sendAddress(_chipAddressArray[chipNumber]);
        if(status == ERROR) {
            ret = status;
        }
    
        status = sendData(startRegister);
        if(status == ERROR) {
            ret = status;
        }
    
        uint32_t ticks = System::ticks();
        while(!LL_I2C_IsActiveFlag_TXE(LED_I2C) && (System::ticks() - ticks) < 2) {
            if(LL_I2C_IsActiveFlag_AF(LED_I2C)) {
                ret = ERROR;
            }
        }

        if(ret == ERROR || !LL_I2C_IsActiveFlag_TXE(LED_I2C)) {
            DBG("TX buffer not empty, aborting");
            return ERROR;
        }

        LL_DMA_ConfigAddresses(
            DMA1,
            LL_DMA_STREAM_7,
            (uint32_t)&_pwmLeds[chipNumber],
            (uint32_t)LL_I2C_DMA_GetRegAddr(LED_I2C),
            LL_DMA_DIRECTION_MEMORY_TO_PERIPH);
        LL_DMA_SetDataLength(DMA1, LL_DMA_STREAM_7, count);
        LL_DMA_EnableIT_TC(DMA1, LL_DMA_STREAM_7);
        LL_DMA_EnableIT_TE(DMA1, LL_DMA_STREAM_7);
        LL_DMA_EnableStream(DMA1, LL_DMA_STREAM_7);

        LL_I2C_EnableDMAReq_TX(LED_I2C);

    } else {
        ret = status;
    }

    //stopTransfer();

    return ret;
}

void LEDDriver::checkStatus() {
    if(LL_I2C_IsActiveFlag_SB(LED_I2C))    { DBG("SB active"); }
    if(LL_I2C_IsActiveFlag_ADDR(LED_I2C))  { DBG("ADDR active"); }
    if(LL_I2C_IsActiveFlag_BTF(LED_I2C))   { DBG("BTF active"); }
    if(LL_I2C_IsActiveFlag_ADD10(LED_I2C)) { DBG("ADD10 active"); }
    if(LL_I2C_IsActiveFlag_STOP(LED_I2C))  { DBG("STOPF active"); }
    if(LL_I2C_IsActiveFlag_RXNE(LED_I2C))  { DBG("RxNE active"); }
    if(LL_I2C_IsActiveFlag_TXE(LED_I2C))   { DBG("TxE active"); }
    if(LL_I2C_IsActiveFlag_BERR(LED_I2C))  { DBG("BERR active"); }
    if(LL_I2C_IsActiveFlag_ARLO(LED_I2C))  { DBG("ARLO active"); }
    if(LL_I2C_IsActiveFlag_AF(LED_I2C))    { DBG("AF active"); }
    if(LL_I2C_IsActiveFlag_OVR(LED_I2C))   { DBG("OVR active"); }
}

ErrorStatus LEDDriver::startTransfer() {
    //DBG("start transfer");
    ErrorStatus ret = SUCCESS;

    // wait for device to become ready
    while(LL_I2C_IsActiveFlag_BUSY(LED_I2C));
    uint32_t ticks = System::ticks();

    LL_I2C_GenerateStartCondition(LED_I2C);

    while(!LL_I2C_IsActiveFlag_SB(LED_I2C) && (System::ticks() - ticks) < 2);
    if(!LL_I2C_IsActiveFlag_SB(LED_I2C)) {
        ret = ERROR;
    }

    return ret;
}

ErrorStatus LEDDriver::stopTransfer() {
    //DBG("stop transfer");
    ErrorStatus ret = SUCCESS;

    // wait until last byte transfer has finished
    uint32_t ticks = System::ticks();
    while(!LL_I2C_IsActiveFlag_BTF(LED_I2C) && (System::ticks() - ticks) < 2) {
        if(LL_I2C_IsActiveFlag_AF(LED_I2C)) {
            ret = ERROR;
        }
    }
    LL_I2C_GenerateStopCondition(LED_I2C);

    return ret;
}

ErrorStatus LEDDriver::sendAddress(uint8_t chipNumber) {
    //DBG("send address %d", chipNumber);
    ErrorStatus ret = SUCCESS;
    uint8_t address = 0b10000000 | (chipNumber << 1);
    LL_I2C_TransmitData8(LED_I2C, address);
    
    uint32_t ticks = System::ticks();
    while(!LL_I2C_IsActiveFlag_ADDR(LED_I2C) && (System::ticks() - ticks) < 2) {
        if(LL_I2C_IsActiveFlag_AF(LED_I2C)) {
            ret = ERROR;
        }
    };
    
    if(ret == SUCCESS) {
        LL_I2C_ClearFlag_ADDR(LED_I2C);
    }

    return ret;
}

ErrorStatus LEDDriver::sendData(uint8_t data) {
    //DBG("send data");
    ErrorStatus ret = SUCCESS;
    
    uint32_t ticks = System::ticks();
    while(!LL_I2C_IsActiveFlag_TXE(LED_I2C) && (System::ticks() - ticks) < 2) {
        if(LL_I2C_IsActiveFlag_AF(LED_I2C)) {
            ret = ERROR;
        }
    }

    if(ret == ERROR || !LL_I2C_IsActiveFlag_TXE(LED_I2C)) {
        DBG("TX buffer not empty, aborting");
        return ERROR;
    }
    
    LL_I2C_TransmitData8(LED_I2C, data);
    ticks = System::ticks();
    while(!LL_I2C_IsActiveFlag_TXE(LED_I2C) && (System::ticks() - ticks) < 2) {
        if(LL_I2C_IsActiveFlag_AF(LED_I2C)) {
            ret = ERROR;
        }
    }

    return ret;
}

ErrorStatus LEDDriver::sendData(uint8_t* data, uint32_t count) {
    ErrorStatus ret = SUCCESS;
    for(uint32_t i = 0; i < count; ++i) {
        ret = sendData(data[i]);
        if(ret == ERROR) {
            break;
        }
    }

    return ret;
}

ErrorStatus LEDDriver::sendSoftwareReset() {
    ErrorStatus ret = SUCCESS, status;

    status = startTransfer();
    if(status == SUCCESS) {
        for(uint32_t chip = 0; chip < NUM_PWM_LED_CHIPS; ++chip) {
            status = writeSingleRegister(chip + 1, 0x00, 0x06);
            if(status == ERROR) {
                ret = status;
            }
        }
    }

    stopTransfer();
    
    return ret;
}

void LEDDriver::ledEnable() {
    LL_GPIO_ResetOutputPin(LED_OEN_GPIO_Port, LED_OEN_Pin);
}

void LEDDriver::ledDisable() {
    LL_GPIO_SetOutputPin(LED_OEN_GPIO_Port, LED_OEN_Pin);
}

bool LEDDriver::ledEnabled() {
    return LL_GPIO_IsOutputPinSet(LED_OEN_GPIO_Port, LED_OEN_Pin);
}