#pragma once

#include "main.h"
#include <cstdint>
#include <array>

#define PCA9685_MODE1  0x00 // location for Mode1 register address
#define PCA9685_MODE2  0x01 // location for Mode2 reigster address
#define PCA9685_LED0   0x06 // location for start of LED0 registers
#define PRE_SCALE_MODE 0xFE //location for setting prescale (clock speed)

#define NUM_PWM_LED_CHIPS 2
#define NUM_LEDS_PER_CHIP 16

#define LED_I2C        I2C2
#define I2C_SPEEDCLOCK 100000
#define I2C_DUTYCYCLE  LL_I2C_DUTYCYCLE_2
#define LED_ON         0
#define LED_OFF        1

#define MAX_R_VALUE    4095
#define MAX_G_VALUE    4095
#define MAX_B_VALUE    4095

class LEDDriver {
public:
    enum LED_TYPE {
        EMPTY,
        COMMON_CATHODE, // LED, current sink
        COMMON_ANODE    // e.g. TC002-N11AS2XT-RGB, current source
    };
    typedef std::array<uint8_t, NUM_PWM_LED_CHIPS> CHIP_ADDRESS_ARRAY;
    typedef std::array<LED_TYPE, NUM_LEDS_PER_CHIP> LED_TYPE_ARRAY;

    void init();
    void process();
    void notifyTxComplete();
    void notifyTxError();
    void ledEnable();
    void ledDisable();
    bool ledEnabled();
    void clear();
    void setSingleLED(uint8_t led, uint16_t brightness);
    void setColourRGB(uint8_t colourLED, float r, float g, float b);
    void setColourHSV(uint8_t colourLED, float H, float S, float V);

private:
    void resetChip(uint32_t chipNumber);
    ErrorStatus writeSingleRegister(uint8_t chipNumber, uint8_t registerAddress, uint8_t registerValue);
    ErrorStatus writeRegisters(uint8_t chipNumber, uint8_t startRegister, uint8_t* registerValues, uint32_t count);
    ErrorStatus writeRegistersDMA(uint8_t chipNumber, uint8_t startRegister, uint8_t* registerValues, uint32_t count);
    void  checkStatus();
    ErrorStatus startTransfer();
    ErrorStatus stopTransfer();
    ErrorStatus sendAddress(uint8_t chipNumber);
    ErrorStatus sendData(uint8_t data);
    ErrorStatus sendData(uint8_t* data, uint32_t lenData);
    ErrorStatus sendSoftwareReset();
    
    uint8_t LEDDriver_get_cur_buf(void);
    uint8_t LEDDriver_get_cur_chip(void);

    uint32_t LEDDriver_init_dma(uint8_t numdrivers, uint8_t *led_image1, uint8_t *led_image2);
    uint32_t LEDDriver_init_direct(uint8_t numdrivers);

    uint8_t get_red_led_element_id(uint8_t rgb_led_id);
    uint8_t get_chip_num(uint8_t rgb_led_id);

    uint8_t _transmissionBusy;
    uint8_t _curChip;
    uint16_t _pwmLeds[NUM_PWM_LED_CHIPS][NUM_LEDS_PER_CHIP][2];
    CHIP_ADDRESS_ARRAY _chipAddressArray;
    LED_TYPE_ARRAY _ledTypeArray[NUM_PWM_LED_CHIPS];

};