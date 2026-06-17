#pragma once

#include <RtcDS1302.h>
#include <ThreeWire.h>
#include <SD.h>

#include "config/pins.h"

namespace SDLogger {
    // Parecida com a de telemetria
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

        float dutyNowPct; // vem de ramp
        float dutyTargetPct; // vem de ramp
        uint8_t  maxPct;  // vem de ramp
    };

    void setup();
    void loop(const Data& data);
}