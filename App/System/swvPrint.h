#include "main.h"
#include <stdio.h>

#define SWV_PRINTBUFFER_SIZE     50

#ifdef __cplusplus
extern "C" {
#endif

// TODO: add global debug enabling/disabling option
#define DBG(_fmt_, ...) printf(_fmt_ "\n", ##__VA_ARGS__)

void USBDBG(const char* fmt, ...);

int __io_putchar(int ch);

#ifdef __cplusplus
}
#endif
