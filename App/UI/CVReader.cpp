#include "CVReader.h"
#include "Utils.h"
#include "main.h"
#include <cstdio>

void CVReader::init() {
    for(int i = 0; i < CONFIG_NUM_POTS; ++i) {
        _potInputBuffer[i].init();
        _potValueLp1[i] = 0.f;
    }
}

void CVReader::push(size_t index, uint16_t value) {
    if(index >= CONFIG_NUM_POTS) {
        return;
    }

    if(!_potInputBuffer[index].full()) {
        _potInputBuffer[index].write(value);
    }
}

bool CVReader::pull(size_t index, uint16_t &value) {
    if(index >= CONFIG_NUM_POTS) {
        return false;
    }
    
    bool result = false;
    if(_potInputBuffer[index].readable()) {
        value = _potInputBuffer[index].read();
        result = true;
    }

    return result;
}

float CVReader::getCV(size_t index) {
    if(index >= CONFIG_NUM_POTS) {
        return -1.f;
    }
    
    uint16_t rawValue;
    //DBG("readable: %d", _potInputBuffer[index].readable());
    while(_potInputBuffer[index].readable()) {
        rawValue = _potInputBuffer[index].read();
        SINGLE_POLE((rawValue * _sampleBase), _potValueLp1[index], 0.025f);
        CONSTRAIN(_potValueLp1[index], 0.0f, 1.0f);
    }

    //DBG("%.2f", _potValueLp1[index]);
    return _potValueLp1[index];

}