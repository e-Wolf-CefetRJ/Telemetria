#include "ramp.h"

namespace {
    struct Config {
        uint16_t rapidRamp = 250;
        float    rapidUp   = 150.0f;
        float    slewUp    = 40.0f;
        float    slewDown  = 60.0f;
        uint8_t  startMin  = 8;
        uint8_t  maxPct    = 100;
        bool     stepMode  = false;
        bool     accelSmoothing = true;
        uint16_t rampDelayMs = 0;
    } config;

    struct Data {
        bool  overrideEnable = false;
        float overridePct    = 0;
        float dutyNowPct     = 0.0f;
        float dutyTargetPct  = 0.0f;
    } data;

    // Defaults
    constexpr uint16_t DEFAULT_RAPID_RAMP_MS    = 250;
    constexpr float    DEFAULT_RAPID_UP_PCTPS   = 150.0f;
    constexpr float    DEFAULT_SLEW_UP_PCTPS    = 40.0f;
    constexpr float    DEFAULT_SLEW_DOWN_PCTPS  = 60.0f;
    constexpr uint8_t  DEFAULT_START_MIN_PCT    = 8;

    constexpr uint8_t  DEFAULT_MAX_PCT          = 100;

    // Constantes
    constexpr uint8_t  DEADZONE_PWM        = 4;  
    constexpr uint8_t  MIN_START_PWM       = 20;
    constexpr uint16_t REST_DEBOUNCE_MS    = 120;
    constexpr uint16_t SOFT_RAMP_MS        = 1500;
    constexpr uint16_t ACCEL_RAMP_MS       = 250;
    constexpr uint8_t  REST_PCT_THRESHOLD  = 2;     

    enum class RampMode : uint8_t {
        Rest,       // Repouso
        SoftStart,  // Partida Suave
        Accel,      // Aceleração Rápida
        Default     // Normal (Aceleração linear ou Desaceleração)
    };

    struct RampState {
        RampMode mode = RampMode::Rest;
        uint32_t restStartMs = 0;
        uint32_t softStartMs = 0;
        uint32_t accelStartMs = 0;
        float    accelFromPct = 0.0f;
        float    accelToPct = 0.0f;
        float    lastPedalPct = 0.0f;
    };
    RampState state;

    // ==============================================

    // Pedal
    uint8_t discretizePedal(float pedalPct) {
        // Conversão de PCT para valor inteiro de 8 bits (máximo 255)
        uint8_t duty = (uint8_t)lroundf(constrain(pedalPct, 0.0f, 100.0f) * 255.0f / 100.0f);
        // Ajustes
        if (duty < DEADZONE_PWM) 
            duty = 0;
        if (duty > 0 && duty < MIN_START_PWM) 
            duty = 0;
        return duty;
    }

    float applyMaxPct(float pctAfter) {
        return pctAfter * ((float) Ramp::getMaxPct() / 100.0f);
    }

    // ==============================================

    // Soft e Rest
    // Define o estado inicial da Rampa: Rest ou SoftStart
    void updateRampMode(float pedalPct, uint8_t pedalDuty8, uint32_t now) {
        bool belowRest = (pedalPct <= REST_PCT_THRESHOLD);
        bool pedalZero = (pedalDuty8 == 0);

        if (belowRest || pedalZero) {
            // Pedal no repouso (pressionado menos que 3%)
            if (state.mode != RampMode::Rest) {
                state.restStartMs = now;
            }
            if ((now - state.restStartMs) >= REST_DEBOUNCE_MS) {
                state.mode = RampMode::Rest;
            }
        } else {
            // Pedal foi pisado (mais que 3%)
            if (state.mode == RampMode::Rest) {
                state.mode = RampMode::SoftStart;
                state.softStartMs = now;
            }
        }
    }

    // Aplica o soft start
    float applySoftCap(float pedalAfter, uint32_t now) {
        if (state.mode != RampMode::SoftStart) return pedalAfter;

        float x = constrain((float)(now - state.softStartMs) / (float)SOFT_RAMP_MS, 0.0f, 1.0f);
        float s = (3.0f * x * x) - (2.0f * x * x * x);   // easing cubic
        float softCapPct = s * 100.0f;

        float capped = min(pedalAfter, softCapPct);
        if (x >= 1.0f) {
            state.mode = RampMode::Default;   // soft terminou
        }
        return capped;
    }

    // ==============================================

    // Aceleração
    void detectAccelRamp(float pedalAfter, uint32_t now) {
        if (!Ramp::getAccelSmoothing() || state.mode == RampMode::SoftStart) {
            state.mode = (state.mode == RampMode::Accel) ? 
                RampMode::Default : // Se já estava em aceleração, volta ao modo normal
                state.mode; 
            return;
        }

        if (pedalAfter > state.lastPedalPct + 0.001f) {
            // Acelerando
            state.mode = RampMode::Accel;
            state.accelStartMs = now;
            state.accelFromPct = Ramp::getDutyNow();
            state.accelToPct   = pedalAfter;
        } else if (pedalAfter + 0.001f < state.lastPedalPct) {
            // Desacelerando
            if (state.mode == RampMode::Accel) 
                state.mode = RampMode::Default;
        }
        state.lastPedalPct = pedalAfter;
    }

    float applyAccelCap(float cappedPct, uint32_t now) {
        if (state.mode != RampMode::Accel) return cappedPct;

        float x = constrain((float)(now - state.accelStartMs) / (float)ACCEL_RAMP_MS, 0.0f, 1.0f);
        float s = (3.0f * x * x) - (2.0f * x * x * x);
        float capPct = state.accelFromPct + s * (state.accelToPct - state.accelFromPct);

        float result = min(cappedPct, capPct);
        if (x >= 1.0f) {
            state.mode = RampMode::Default;
        }
        return result;
    }

    // Alvo final
    float calculateTargetPct(float cappedPct, float pedalPct) {
        if (Ramp::getOverrideEnable()) {
            return (Ramp::getOverridePct() > 0) ? (float) Ramp::getOverridePct() : 0.0f;
        }
        float startMin = (pedalPct > 0.0f && pedalPct < Ramp::getStartMin()) ? (float)Ramp::getStartMin() : 0.0f;
        return max(cappedPct, startMin);
    }

    // Aplicação
    void applySlewRate(uint32_t now, uint32_t dt_ms) {
        static float lastTargetLegacy = 0.0f;
        static uint32_t rapidUntilMs = 0;

        float dutyNowPct = Ramp::getDutyNow();
        float targetPct = Ramp::getDutyTarget();

        if (targetPct > lastTargetLegacy + 0.5f) {
            rapidUntilMs = now + Ramp::getRapidRamp();
        }
        lastTargetLegacy = targetPct;

        float slewUp = Ramp::getSlewUp();
        if (rapidUntilMs > now && targetPct > dutyNowPct) {
            slewUp = max(slewUp, Ramp::getRapidUp());
        }

        float stepUp = (slewUp * dt_ms) / 1000.0f;
        float stepDn = (Ramp::getSlewDown() * dt_ms) / 1000.0f;

        if (dutyNowPct < targetPct) {
            dutyNowPct = min(targetPct, dutyNowPct + stepUp);
        } else if (dutyNowPct > targetPct) {
            dutyNowPct = max(targetPct, dutyNowPct - stepDn);
        }
        Ramp::setDutyNow(dutyNowPct);
    }

    void applyStepMode() {
        uint8_t cur8 = (uint8_t)lroundf(constrain(Ramp::getDutyNow(), 0.0f, 100.0f) * 255.0f / 100.0f);
        uint8_t tgt8 = (uint8_t)lroundf(constrain(Ramp::getDutyTarget(), 0.0f, 100.0f) * 255.0f / 100.0f);
        int diff8 = (int)tgt8 - (int)cur8;
        int step = 2 + abs(diff8) / 8;
        if (step > 32) step = 32;

        if (diff8 > 0) {
            uint16_t next = cur8 + step;
            if (next > tgt8) next = tgt8;
            cur8 = (uint8_t)next;
        } else if (diff8 < 0) {
            int next = (int)cur8 - step;
            if (next < tgt8) next = tgt8;
            cur8 = (uint8_t)next;
        }

        float dutyNowPct = (cur8 * 100.0f) / 255.0f;
        Ramp::setDutyNow(dutyNowPct);
    }
}

namespace Ramp {
    void defaultValue() {
        setRapidRamp(DEFAULT_RAPID_RAMP_MS);
        setRapidUp(DEFAULT_RAPID_UP_PCTPS);
        setSlewUp(DEFAULT_SLEW_UP_PCTPS);
        setSlewDown(DEFAULT_SLEW_DOWN_PCTPS);
        setStartMin(DEFAULT_START_MIN_PCT);
        setMaxPct(DEFAULT_MAX_PCT);
    }

    // Controle
    void start() {
        setOverrideEnable(false);
        setOverridePct(0);
    }

    void hold(int pct) {
        setOverrideEnable(true);
        setOverridePct(constrain(pct,0,100));
        setDutyTarget(pct);
    }

    void stop() {
        setOverrideEnable(true);
        setOverridePct(0);
        setDutyTarget(0);
    }

    // Main
    void setup() {
        state.mode = RampMode::Rest;
       
        state.softStartMs = 0;

        setDutyNow(0);
        setDutyTarget(0);

        state.accelStartMs = 0;
        state.accelFromPct = 0.0f;
        state.accelToPct = 0.0f;
        state.lastPedalPct = 0.0f;
    }

    void loop(float pedalPct, uint32_t now, uint32_t dt_ms) {
        uint8_t pedalDuty8 = discretizePedal(pedalPct);
        float pedalPctAfter = (pedalDuty8 * 100.0f) / 255.0f; 
        pedalPctAfter = applyMaxPct(pedalPctAfter); 

        /*
            PedalDuty8 = Valor inteiro de 8 bits (bruto PWM), valores menores que 20 levam valor 0, e seu máximo é 255
            PedalPCTAfter = Percentual após tratamento, valores menores que 7,8% tem valor 0, e seu máximo é 100%
            PedalPct = Percentual de 0 à 100%
        */

        // Rest e Soft
        updateRampMode(pedalPct, pedalDuty8, now);
        float cappedPct = applySoftCap(pedalPctAfter, now);

        // Rampa S de Aceleração
        detectAccelRamp(pedalPctAfter, now);
        cappedPct = applyAccelCap(cappedPct, now);

        // Alvo final
        float dutyTargetPct = calculateTargetPct(cappedPct, pedalPct);
        setDutyTarget(dutyTargetPct);

        // Aplicação
        if (!getStepMode()) {
            applySlewRate(now, dt_ms);
        } else {
            applyStepMode();
        }
    }

    // Getters
    uint16_t getRapidRamp()    { return config.rapidRamp; }
    float    getRapidUp()      { return config.rapidUp; }
    float    getSlewUp()       { return config.slewUp; }
    float    getSlewDown()     { return config.slewDown; }
    uint8_t  getStartMin()     { return config.startMin; }
    uint8_t  getMaxPct()       { return config.maxPct; }
    bool     getStepMode()     { return config.stepMode; }
    bool     getAccelSmoothing() { return config.accelSmoothing; }
    uint16_t getRampDelayMs()   { return config.rampDelayMs; }

    bool     getOverrideEnable() { return data.overrideEnable; }
    float    getOverridePct()    { return data.overridePct; }
    float    getDutyNow()        { return data.dutyNowPct; }
    float    getDutyTarget()     { return data.dutyTargetPct; }

    // Setters
    void setRapidRamp(uint16_t ms)   { config.rapidRamp = ms; }
    void setRapidUp(float pctps)     { config.rapidUp = pctps; }
    void setSlewUp(float pctps)      { config.slewUp = pctps; }
    void setSlewDown(float pctps)    { config.slewDown = pctps; }
    void setStartMin(uint8_t pct)    { config.startMin = pct; }
    void setMaxPct(uint8_t pct)      { config.maxPct = pct; }
    void setStepMode(bool enable)    { config.stepMode = enable; }
    void setAccelSmoothing(bool enable) { config.accelSmoothing = enable; }
    void setRampDelayMs(uint16_t delay) { config.rampDelayMs = delay; }

    void setOverrideEnable(bool enable) { data.overrideEnable = enable; }
    void setOverridePct(float pct)        { data.overridePct = constrain(pct, 0, 100); }
    
    void setDutyNow(float pct) {
        data.dutyNowPct = constrain(pct, 0.0f, 100.0f);
    }
    
    void setDutyTarget(float pct) {
        data.dutyTargetPct = constrain(pct, 0.0f, 100.0f);
    }
}