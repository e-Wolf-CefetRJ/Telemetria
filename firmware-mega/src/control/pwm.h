#pragma once

#include "config/pins.h"

#include <Arduino.h>

namespace PWM {
    // Verificar quais funções devem ser públicas ou privadas
    void setup();
    void loop(float dutyNowPct);

    void applyFrequency(uint16_t hz);
    
    void defaultValue();

    void setFrequency(uint16_t hz);
    uint16_t getFrequency();
}