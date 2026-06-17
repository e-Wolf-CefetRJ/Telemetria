#include "core/telemetry.h"

// Sensores
#include "sensors/current/current.h"
#include "sensors/throttle/throttle.h"

// Controle
#include "control/ramp.h"

namespace Telemetry {
    uint8_t printmode = 0;

    void loop(const Data& data) {
        if (printmode == 0) {
            Serial.print(F("MS:"));    Serial.print(data.now);          Serial.print(' ');
            Serial.print(F("V:"));     Serial.print(data.volts, 3);    Serial.print(' ');
            Serial.print(F("Pct:"));   Serial.print(data.pct, 1);      Serial.print(' ');
            Serial.print(F("Temp:"));  
            if (isnan(data.temp)) Serial.print(F("NaN")); else Serial.print(data.temp, 1);
            Serial.print(' ');
            Serial.print(F("Humi:"));  
            if (isnan(data.humi)) Serial.print(F("NaN")); else Serial.print(data.humi, 1);
            Serial.print(' ');
            Serial.print(F("RPM:"));   Serial.print(data.rpm, 1);      Serial.print(' ');
            Serial.print(F("Speed:")); Serial.print(data.speedKmh, 2); Serial.print(' ');
            Serial.print(F("IBAT:"));  Serial.print(data.currentBat, 2); Serial.print(' ');
            Serial.print(F("IMOT:"));  Serial.print(data.currentMot, 2); Serial.print(' ');
            Serial.print(F("MIN:"));   Serial.print(data.voltageMin, 3);    Serial.print(' ');
            Serial.print(F("MAX:"));   Serial.print(data.voltageMax, 3);    Serial.print(' ');
            Serial.print(F("WHEEL:")); Serial.print(data.wheelCm, 1); Serial.print(' ');
            Serial.print(F("PPR:"));   Serial.print(data.ppr);         Serial.print(' ');
            Serial.print(F("OVR:"));   Serial.print(data.overrideEnabled ? 1 : 0); Serial.print(' ');
            Serial.print(F("OVRPCT:"));Serial.print(data.overridePct);    Serial.print(' ');
            Serial.print(F("MAXPCT:"));Serial.print(data.maxPct);     Serial.println();
        }
    }

    void setPrintMode(uint8_t value) {
        printmode = value;
    }
    uint8_t getPrintMode() {
        return printmode;
    }
}