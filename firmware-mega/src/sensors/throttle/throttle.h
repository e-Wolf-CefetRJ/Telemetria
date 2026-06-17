#pragma once

#include <Arduino.h>

#include "../hal/hal_adc.h"
#include "config/pins.h"

namespace Throttle {
    void defaultValue();
    void loop();

    // GETTTERS
    float getVolts();
    float getPct();

    float getVoltageMin();
    float getVoltageMax();

    // SETTERS
    void setVoltageMin(float value);
    void setVoltageMax(float value);
}   