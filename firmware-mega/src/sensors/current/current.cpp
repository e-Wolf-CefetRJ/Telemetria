#include "current.h"

// Valores privados
namespace {
    // calib ACS712
    constexpr float ACS_MV_PER_A = 100.0f; // milivolts por ampere
    constexpr float ACS_V_OFFSET = 2.5f;   // valor da voltagem quando corrente é nula

    // Data
    struct Data {
        float currentBattery = 0.0f;
        float currentMotor = 0.0f;
    };
    Data data;

    // State (Bateria e Motor)
    struct CurrentState {
        float currentOffset   = 0.0f;
        bool  offsetValid     = false;
        float iir             = 0.0f;  // Corrente com filtragem
    };
    CurrentState batteryState;
    CurrentState motorState;

    constexpr float ALPHA = 0.2f;
    constexpr float BETA  = 0.02f;  

    // Insertion Sort
    void sortSamples(int n, int samples[]) {
        for (int i = 1; i < n; i++){
            int temp = samples[i];
            int key = i - 1;

            while (key >= 0 && temp < samples[key]) {
                samples[key + 1] = samples[key];
                key--;
            }
            samples[key+1] = temp;
        }
        
    }

    float getFilteredCurrent(uint8_t pin, CurrentState &state, bool idle) {
        const uint8_t SAMPLES_NUMBER = 7;
        int samples[SAMPLES_NUMBER];

        for (int i = 0; i < SAMPLES_NUMBER; i++) {
            (void) analogRead(pin);
            samples[i] = analogRead(pin);
        }

        sortSamples(SAMPLES_NUMBER, samples);

        int raw = samples[SAMPLES_NUMBER/2]; // Mediana

        float voltage = (raw * HAL::Adc::VREF_VOLTS) / HAL::Adc::MAX_VALUE;
        float current = ((voltage - ACS_V_OFFSET) * 1000.0f) / ACS_MV_PER_A;
        
        state.iir = (1-ALPHA) * state.iir + ALPHA * current;

        if (idle) {
            state.currentOffset = (1-BETA) * state.currentOffset + BETA * state.iir;
            state.offsetValid = true;
        }
        
        float filteredCurrent = state.iir - (state.offsetValid ? state.currentOffset : 0.0f);

        if (fabs(filteredCurrent) < 0.08f)
            filteredCurrent = 0.0f;

        return filteredCurrent;
    }
}

namespace Current {
    void loop(bool idle) {
        data.currentBattery = getFilteredCurrent(Pins::IBAT, batteryState, idle);
        data.currentMotor   = getFilteredCurrent(Pins::IMOT, motorState, idle);
    }

    float getCurrentBattery() {
        return data.currentBattery;
    }

    float getCurrentMotor() {
        return data.currentMotor;
    }
}