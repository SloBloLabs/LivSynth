#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void testMain(void);
void appMain(void);
void appTick(void);
void appClockTimer();
void appADCCompleteRequest();
void appLEDTxComplete();
void appLEDTxError();
float adc2bpm(uint16_t adcValue);
float adc2Volt(uint16_t adcValue);
void startSequencer();
void stopSequencer();
void setTempo();
void setPitch();

#ifdef __cplusplus
}
#endif
