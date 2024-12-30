#include "swvPrint.h"
#include <stdarg.h>
#include "usbd_cdc_if.h"

static char printBuffer[SWV_PRINTBUFFER_SIZE];

void UDBG(const char* fmt, ...) {
    
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

// for graph debugging
uint8_t ITM_SendChar(uint8_t ch, size_t const port)
{
  if (((ITM->TCR & ITM_TCR_ITMENA_Msk) != 0UL) &&      /* ITM enabled */
      ((ITM->TER & (port+1)          ) != 0UL)   )     /* ITM Port enabled */
  {
    while (ITM->PORT[port].u32 == 0UL)
    {
      __NOP();
    }
    ITM->PORT[port].u8 = (uint8_t)ch;
   }
  return (ch);
}
