#include "ShiftRegister.h"
#include "main.h"

#define SR_SPI SPI3

ShiftRegister::ShiftRegister() {
    _input = 0;
    _output = 0;
}

void ShiftRegister::init() {

    // Start SPI
    // DMA is pretty overkill, move polling to main function + process method
    /*LL_DMA_ConfigAddresses(
        DMA1,
        LL_DMA_STREAM_0,
        LL_SPI_DMA_GetRegAddr(SPI3),
        (uint32_t)&_buttonState,
        LL_DMA_GetDataTransferDirection(DMA1, LL_DMA_STREAM_0)
    );
    LL_DMA_SetDataLength(DMA1, LL_DMA_STREAM_0, NUM_SHIFT_REGISTERS);
    LL_DMA_EnableIT_TC(DMA1, LL_DMA_STREAM_0);*/

    //LL_GPIO_ResetOutputPin(GPIOD, LL_GPIO_PIN_2);
    //LL_GPIO_SetOutputPin(GPIOD, LL_GPIO_PIN_2);

    //LL_SPI_EnableDMAReq_RX(SPI3);
    LL_SPI_Enable(SR_SPI);
    //LL_DMA_EnableStream(DMA1, LL_DMA_STREAM_0);
}

void ShiftRegister::process() {
    // SR Load
    LL_GPIO_ResetOutputPin(SR_SHLDN_GPIO_Port, SR_SHLDN_Pin);
    // wait a bit
    asm volatile ("nop");
    asm volatile ("nop");
    // SR Shift
    LL_GPIO_SetOutputPin(SR_SHLDN_GPIO_Port, SR_SHLDN_Pin);


    // wait for TXE flag to transmit data or SPI is busy
    while(!LL_SPI_IsActiveFlag_TXE(SR_SPI));
    //LL_SPI_TransmitData8(SR_SPI, _outputs[i]);
    LL_SPI_TransmitData16(SR_SPI, _output);
    // Wait for transfer to finish
    while(!LL_SPI_IsActiveFlag_RXNE(SR_SPI));
    //while (!(SR_SPI->SR & SPI_SR_RXNE));
    // Read data from DR
    _input = LL_SPI_ReceiveData16(SR_SPI);

}