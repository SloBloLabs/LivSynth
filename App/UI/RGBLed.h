#pragma once

#include "Config.h"

#include "swvPrint.h"

class RGBLed {
public:
    enum Code {
        None  = -1,
        Tune  = 0,
        Step1 = 1,
        Step2 = 2,
        Step3 = 3,
        Step4 = 4,
        Step5 = 5,
        Step6 = 6,
        Step7 = 7,
        Step8 = 8,
        Play  = 9
    };
};