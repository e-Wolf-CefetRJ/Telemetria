#include "rpm.h"

// mudar unsigned long para uint32_t ou uint64_t
namespace {
    struct Data {
        float wheelCm = 50.8f;
        uint8_t ppr = 1;

        float rpm = 0;
        float speed = 0;
    }; 

    struct IsrData {
        volatile unsigned long lastMeasureUs = 0;
        volatile unsigned long periodUs = 700000UL;     // Period Between Pulses
        volatile unsigned long periodAvgUs = 700000UL;  // Period Avarage
        volatile unsigned long periodSumUs = 0;
        volatile unsigned int pulseCounter = 1;
        volatile uint8_t lastState = 0;
        volatile unsigned int amountOfReadings = 1;
    };

    struct RuntimeData {
        unsigned long frequencyRaw = 0;
        unsigned int zeroDebounceExtra = 0;
        unsigned long zeroTimeoutUs = 600000UL;
        volatile unsigned long minPulseUs = 200;

        static constexpr byte numReadings = 2;

        unsigned long readings[numReadings] = {0};
        unsigned long readIndex = 0;
        unsigned long total = 0;
    };

    struct Snapshot {
        unsigned long lastMeasureUs;
        unsigned long periodUs;
        unsigned long periodAvgUs;
    };

    Data data;
    IsrData isr;
    RuntimeData runtime;

    float getCircumference(){ return 3.14159265f * (RPM::getWheelCm()*0.01f); } // Circunferência em metros
    float rpmToKmh(){ return RPM::getRpm() * getCircumference() * 0.06f; }

    void setSpeed(float speed) { data.speed = speed; }
    void setRpm(float avg) { data.rpm = avg; }

    Snapshot snapshot() {
        noInterrupts();

        Snapshot s = {
            isr.lastMeasureUs,
            isr.periodUs,
            isr.periodAvgUs
        };

        interrupts();

        return s;
    }
}

// Toda variável alterada aqui deve ser volatile
ISR(PCINT2_vect) {
    uint8_t state = PINK & Pins::PCINT_PIN;

    if(state && !isr.lastState) {
        unsigned long now = micros();
        unsigned long pbp = now - isr.lastMeasureUs;

        if(pbp >= runtime.minPulseUs) {
            isr.periodUs = pbp;
            isr.lastMeasureUs = now;

            if(isr.pulseCounter >= isr.amountOfReadings) {
                isr.periodAvgUs = isr.periodSumUs / isr.amountOfReadings;
                isr.pulseCounter = 1;
                isr.periodSumUs = isr.periodUs;

                int remapped = map((int)isr.periodUs, 40000, 5000, 1, 10);
                remapped = constrain(remapped, 1, 10);

                isr.amountOfReadings = remapped;
            } else {
                isr.pulseCounter++;
                isr.periodSumUs += isr.periodUs;
            }
        }
    }   

    isr.lastState = state;
}

namespace RPM {
    void setup() {
        pinMode(Pins::RPM, INPUT_PULLUP);
        isr.lastState = PINK & Pins::PCINT_PIN;
        PCICR |= Pins::PCIE_NUM;
        PCMSK2 |= Pins::PCINT_MASK;
    }

    void loop() {
        Snapshot snap = snapshot();

        unsigned long CurrentMicros=micros();

        if(CurrentMicros < snap.lastMeasureUs) 
            snap.lastMeasureUs = CurrentMicros;

        if(snap.periodUs > runtime.zeroTimeoutUs - runtime.zeroDebounceExtra ||
            CurrentMicros - snap.lastMeasureUs > runtime.zeroTimeoutUs - runtime.zeroDebounceExtra) {
            
            runtime.frequencyRaw = 0; 
            runtime.zeroDebounceExtra = 2000;

        } else {
            runtime.zeroDebounceExtra = 0;
            runtime.frequencyRaw = 10000000000UL / (snap.periodAvgUs ? snap.periodAvgUs : 1);
        }

        unsigned long rpm_inst=0; 

        uint8_t ppr = (getPpr() == 0) ? 1 : getPpr();

        if(runtime.frequencyRaw>0) { 
            rpm_inst=(runtime.frequencyRaw*60UL)/ppr; 
            rpm_inst/=10000UL; 
        }

        runtime.total -= runtime.readings[runtime.readIndex]; 
        runtime.readings[runtime.readIndex] = rpm_inst; 

        runtime.total += runtime.readings[runtime.readIndex];
        runtime.readIndex = (runtime.readIndex+1) % runtime.numReadings; 

        float avg = (float) runtime.total / runtime.numReadings;

        setRpm(avg); 
        setSpeed(rpmToKmh());
    }

    // Getters
    float getWheelCm() { return data.wheelCm; }
    uint8_t getPpr() { return data.ppr; }
    float getRpm() { return data.rpm; }
    float getSpeed() { return data.speed; }

    long getMinPulse() { return runtime.minPulseUs; }

    // Setters
    void setWheelCm(float value) { data.wheelCm = value; }
    void setPpr(uint8_t value) { data.ppr = value; }

    void setMinPulse(long value) { runtime.minPulseUs = value; }
    void setZeroTimeout(unsigned long value) { runtime.zeroTimeoutUs = value; }
} 