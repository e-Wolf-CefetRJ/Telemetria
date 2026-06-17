#pragma once

#include <Arduino.h>

#include "../hal/hal_adc.h"
#include "config/pins.h"

namespace Current {
    // idle = (dutyNowPct < 1.0f) && (rpm < 5.0f)
    void loop(bool idle);
    
    float getCurrentBattery();
    float getCurrentMotor();
}