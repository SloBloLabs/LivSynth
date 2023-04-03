#pragma once

#include "main.h"

class Flash {
public:

    static inline void init() {
        __HAL_FLASH_SET_LATENCY(5);
        __HAL_FLASH_PREFETCH_BUFFER_ENABLE();
        __HAL_FLASH_DATA_CACHE_ENABLE();
    }

    static inline void unlock() {
        HAL_FLASH_Unlock();
    }

    static inline void lock() {
        HAL_FLASH_Lock();
    }

    static inline void eraseSector(uint32_t sector) {
        FLASH_Erase_Sector(sector, FLASH_VOLTAGE_RANGE_3);
    }

    static inline void program(uint32_t address, uint32_t data) {
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address, data);
    }
};
