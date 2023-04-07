#pragma once

#include "Flash.h"
#include "swvPrint.h"

#include <cstring>
#include <algorithm>

template<uint32_t> struct Sector {};

// bank1
template<> struct Sector<0>  { enum { start = 0x08000000 }; }; //  16kB
template<> struct Sector<1>  { enum { start = 0x08004000 }; }; //  16kB
template<> struct Sector<2>  { enum { start = 0x08008000 }; }; //  16kB
template<> struct Sector<3>  { enum { start = 0x0800C000 }; }; //  16kB
template<> struct Sector<4>  { enum { start = 0x08010000 }; }; //  64kB
template<> struct Sector<5>  { enum { start = 0x08020000 }; }; // 128kB
template<> struct Sector<6>  { enum { start = 0x08040000 }; }; // 128kB
template<> struct Sector<7>  { enum { start = 0x08060000 }; }; // 128kB
template<> struct Sector<8>  { enum { start = 0x08080000 }; }; // 128kB
template<> struct Sector<9>  { enum { start = 0x080A0000 }; }; // 128kB
template<> struct Sector<10> { enum { start = 0x080C0000 }; }; // 128kB
template<> struct Sector<11> { enum { start = 0x080E0000 }; }; // 128kB
// bank2
template<> struct Sector<12> { enum { start = 0x08100000 }; }; //  16kB
//...

template<uint32_t sectorIndex>
class FlashStorage {
public:

    template<typename T>
    void write(const T &data) {
        Flash::unlock();
        
        Flash::eraseSector(sectorIndex);

        size_t size = std::min((sizeof(T) + 3) & ~0x03, (unsigned int)FLASH_STORAGE_SIZE);
        DBG("Write Model: Copy %d bytes from %p to %p", size, (void *)&data, (void *)FLASH_STORAGE_BASE);

        //const uint32_t *src = (const uint32_t *)(&data);
        const uint32_t *src = reinterpret_cast<const uint32_t *>(&data);
        uint32_t address = FLASH_STORAGE_BASE;
        while(size) {
            //DBG("writing data 0x%.8lx from %p to address %p", *src, (void *)src, (void *)address);
            Flash::program(address, *src);
            address += 4;
            size -= 4;
            ++src;
        }

        Flash::lock();
    }

    template<typename T>
    void read(T &data) {
        size_t size = std::min((sizeof(T) + 3) & ~0x03, (unsigned int)FLASH_STORAGE_SIZE);
        DBG("Load Model: Copy %d bytes from %p to %p", size, (void *)FLASH_STORAGE_BASE, (void *)&data);

        uint32_t *target = reinterpret_cast<uint32_t *>(&data);
        uint32_t address = FLASH_STORAGE_BASE;
        while(size) {
            //DBG("Reading data 0x%.8lx from %p to address %p", *(uint32_t*)address, (void *)address, (void *)target);
            *target = *(uint32_t*)address;
            address += 4;
            size -= 4;
            ++target;
        }
        //memcpy(&data, (const void*)FLASH_STORAGE_BASE, len);
    }

private:
    enum {
        FLASH_STORAGE_BASE = Sector<sectorIndex>::start,
        FLASH_STORAGE_END  = Sector<sectorIndex + 1>::start,
        FLASH_STORAGE_SIZE = FLASH_STORAGE_END - FLASH_STORAGE_BASE
    };
};
