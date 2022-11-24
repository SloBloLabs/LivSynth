#include "AdcInternal.h"
#include "main.h"

#define ADC_POTS ADC1

void AdcInternal::init() {
    // reset; memset not applicable to volitiles
    // std::memset(_channels, 0, sizeof(_channels));
    for(int i = 0; i < NUM_ADC_CHANNELS; ++i) {
        _channels[i] = 0;
    }

    // Start DMA and ADC
    LL_DMA_ConfigAddresses(
        DMA2,
        LL_DMA_STREAM_0,
        LL_ADC_DMA_GetRegAddr(ADC_POTS, LL_ADC_DMA_REG_REGULAR_DATA),
        (uint32_t)_channels,
        LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
    LL_DMA_SetDataLength(DMA2, LL_DMA_STREAM_0, NUM_ADC_CHANNELS);
    // Optional! DMA will transfer even without calling ISR
    // Can be enabled for debugging purposes:
    LL_DMA_EnableIT_TC(DMA2, LL_DMA_STREAM_0);
    LL_DMA_EnableStream(DMA2, LL_DMA_STREAM_0);

    LL_ADC_Enable(ADC_POTS);
    LL_ADC_REG_StartConversionSWStart(ADC_POTS);
}