#include "swvPrint.h"
#include <stdarg.h>
#include "usbd_cdc_if.h"

static char printBuffer[SWV_PRINTBUFFER_SIZE];

void USBDBG(const char* fmt, ...) {
    
    va_list va;
    va_start (va, fmt);
    vsprintf (printBuffer, fmt, va);
    va_end (va);
    
    CDC_Transmit_FS((uint8_t*)printBuffer, strlen(printBuffer));
}

int __io_putchar(int ch) {
    ITM_SendChar(ch);
    return ch;
}