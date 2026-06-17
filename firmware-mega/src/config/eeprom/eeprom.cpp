#include "eeprom.h"

// Fowller-Noll-Vo hash function; Cria uma assinatura a cada compilação
// Única forma da assinatura for igual, é se as compilações forem ao mesmo tempo
uint32_t buildSignature() {
    const char* date = __DATE__;
    const char* time = __TIME__;

    // FNV-1a 32-bit: offset basis e prime
    uint32_t hash = 2166136261UL;   // 0x811c9dc5
    const uint32_t prime = 16777619UL; // 0x01000193

    while (*date) {
        hash ^= (uint8_t)*date++;
        hash *= prime;
    }

    hash ^= (uint8_t)'|';
    hash *= prime;

    while (*time) {
        hash ^= (uint8_t)*time++;
        hash *= prime;
    }

    return hash;
}

namespace Eeprom{
    Config config;
    
    void reset() {
        Throttle::defaultValue();
        Ramp::defaultValue();
        PWM::defaultValue();
    }

    void save() {
        config.firmwareSignature = buildSignature();

        config.voltageMin  =  Throttle::getVoltageMin();
        config.voltageMax  =  Throttle::getVoltageMax();

        config.pwm_hz   =   PWM::getFrequency();

        config.rapid_ms  =   Ramp::getRapidRamp();
        config.rapidUp   =   Ramp::getRapidUp();
        config.slewUp    =   Ramp::getSlewUp();
        config.slewDown  =   Ramp::getSlewDown();
        config.startMin  =   Ramp::getStartMin();
        config.maxPct    =   Ramp::getMaxPct();

        EEPROM.put(0,config);
    }

    bool load() {
        EEPROM.get(0,config);

        if (config.firmwareSignature != buildSignature()) return false;

        Throttle::setVoltageMin(config.voltageMin);
        Throttle::setVoltageMax(config.voltageMax);

        PWM::setFrequency(config.pwm_hz);
        
        Ramp::setRapidRamp(config.rapid_ms);
        Ramp::setRapidUp(config.rapidUp);
        Ramp::setSlewUp(config.slewUp);
        Ramp::setSlewDown(config.slewDown);
        Ramp::setStartMin(config.startMin);
        Ramp::setMaxPct(config.maxPct);
        
        return true;
    }

    void setup() {
        if (!load()) {
            reset();
            save();
        }
    }
}