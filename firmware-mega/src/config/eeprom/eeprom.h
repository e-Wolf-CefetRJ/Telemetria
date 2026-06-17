#pragma once

#include <Arduino.h>
#include <EEPROM.h>

#include "sensors/throttle/throttle.h"
#include "control/ramp.h"
#include "control/pwm.h"

namespace Eeprom {
    struct Config {
        uint32_t firmwareSignature;    // Assinatura que muda a cada compilação
        
        // Acelerador
        float voltageMin; 
        float voltageMax;
        
        // PWM
        uint16_t pwm_hz;
        
        // Rampa
        uint16_t rapid_ms;
        float rapidUp;
        float slewUp;
        float slewDown;
        uint8_t startMin;
        uint8_t maxPct;
    };

    void reset();
    void save();
    bool load();

    void setup();
}