#include "main.h"
#include <stdio.h>

#define SWV_PRINTBUFFER_SIZE     50

#ifdef __cplusplus
extern "C" {
#endif

// TODO: add global debug enabling/disabling option
#define DBG(_fmt_, ...) printf(_fmt_ "\n", ##__VA_ARGS__)

void UDBG(const char* fmt, ...);

int __io_putchar(int ch);

#ifdef __cplusplus
}
#endif

uint8_t ITM_SendChar(uint8_t ch, size_t const port);