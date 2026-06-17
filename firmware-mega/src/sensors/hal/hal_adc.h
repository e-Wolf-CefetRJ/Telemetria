#pragma once

#include <Arduino.h>

// Constantes compartilhadas
namespace HAL::Adc {
    constexpr int   MAX_VALUE  = 1023;
    constexpr float VREF_VOLTS = 5.0f;
}