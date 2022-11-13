#include "swvPrint.h"

int __io_putchar(int ch) {
    ITM_SendChar(ch);
    return ch;
}