#pragma once

#include <Arduino.h>
#include <math.h>

namespace Ramp {
    void defaultValue();

    void start();
    void hold(int pct);
    void stop();

    void setup();
    void loop(float pedalPct, uint32_t now, uint32_t dt_ms);

    // Getters
    // Config
    uint16_t getRapidRamp();
    float    getRapidUp();
    float    getSlewUp();
    float    getSlewDown();
    uint8_t  getStartMin();

    uint8_t  getMaxPct();

    bool     getStepMode();
    bool     getAccelSmoothing();
    uint16_t getRampDelayMs();

    // Data
    bool     getOverrideEnable();
    float    getOverridePct();
    float    getDutyNow();
    float    getDutyTarget();

    // Setters
    // Config
    void setRapidRamp(uint16_t ms);
    void setRapidUp(float pctps);
    void setSlewUp(float pctps);
    void setSlewDown(float pctps);
    void setStartMin(uint8_t pct);

    void setMaxPct(uint8_t pct);

    void setStepMode(bool enable);
    void setAccelSmoothing(bool enable);
    void setRampDelayMs(uint16_t delay);
    // Data
    void setOverrideEnable(bool enable);
    void setOverridePct(float pct);
    void setDutyNow(float pct);
    void setDutyTarget(float pct);
}