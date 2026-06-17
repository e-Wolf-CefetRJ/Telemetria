#include "throttle.h"

// Valores Privados
namespace {
    struct Config {
        float voltageMin = 0.0f;
        float voltageMax = 0.0f;
    };

    struct Data {
        float volts = 0.0f;
        float pct = 0.0f;
    };

    struct InternalData {
        float filtered = 0.0f;
    };

    Config config;
    Data data;
    InternalData internal;

    // Defaults
    constexpr float VOLTAGE_MIN = 1.10f;
    constexpr float VOLTAGE_MAX = 4.25f;

    // Fault
    constexpr float VOLTAGE_FAULT_LOW  = 0.30f;
    constexpr float VOLTAGE_FAULT_HIGH = 4.90f;

    float readFilteredVoltage(uint8_t pin) {
        int raw = analogRead(pin);

        internal.filtered = (internal.filtered*3 + raw*5) / 8;
        float v_adc = (internal.filtered * HAL::Adc::VREF_VOLTS) / HAL::Adc::MAX_VALUE;
        return v_adc;
    }

    float voltageToPct(float v) {
        float pct = (v - config.voltageMin) / (config.voltageMax - config.voltageMin);

        if (!isfinite(pct))
            pct = 0.0f;

        return constrain(pct, 0.0f, 1.0f) * 100.0f; // Valor percentual
    }

    void setData() {
        float voltage = readFilteredVoltage(Pins::THROTTLE);
        
        bool fault = (voltage < VOLTAGE_FAULT_LOW) || (voltage > VOLTAGE_FAULT_HIGH);
        float pedalPct = fault ? 0.0f : voltageToPct(voltage);
        
        data.volts = voltage;
        data.pct = pedalPct;
    }
}

namespace Throttle {
    void loop() {
        setData();
    }

    float getVolts() {
        return data.volts;
    }

    float getPct() {
        return data.pct;
    }

    float getVoltageMin() {
        return config.voltageMin;
    }

    float getVoltageMax() {
        return config.voltageMax;
    }

    void setVoltageMin(float value) {
        config.voltageMin = value;
    }
    
    void setVoltageMax(float value) {
        config.voltageMax = value;
    }

    void defaultValue() {
        setVoltageMin(VOLTAGE_MIN);
        setVoltageMax(VOLTAGE_MAX);
    }
}