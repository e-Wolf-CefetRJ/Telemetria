#pragma once

#include <Arduino.h>

namespace Telemetry {    
    struct Data {
        uint32_t now;

        // Sensors
        float volts; // vem de throttle
        float pct;   // vem de throttle

        float temp;  // vem de dht
        float humi;  // vem de dht

        float rpm;       // vem de rpm
        float speedKmh;  // vem de rpm

        float currentBat; // vem de current
        float currentMot; // vem de current

        float voltageMin; // vem de throttle
        float voltageMax; // vem de throttle

        float wheelCm;    // vem de rpm
        uint8_t ppr;      // vem de rpm

        // Control
        bool overrideEnabled; // vem de ramp
        float overridePct;    // vem de ramp

        uint8_t  maxPct;  // vem de ramp
    };

    void setPrintMode(uint8_t value);
    uint8_t getPrintMode();

    void loop(const Data& data);
}