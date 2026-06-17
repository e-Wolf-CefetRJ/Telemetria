#pragma once

#include "config/pins.h"

#include <DHT.h>
#include <Arduino.h>
//#include <cmath> importar biblioteca depois

namespace Dht {
    void setup();
    void loop(uint32_t now);
    float getTemp();
    float getHumi();
}