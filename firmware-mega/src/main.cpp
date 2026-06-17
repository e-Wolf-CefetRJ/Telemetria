#include <Arduino.h>

#include "config/eeprom/eeprom.h"

#include "control/pwm.h"
#include "control/ramp.h"

#include "core/telemetry.h"
#include "core/commands.h"

#include "sensors/throttle/throttle.h"
#include "sensors/current/current.h"
#include "sensors/dht/dht.h"
#include "sensors/rpm/rpm.h"

#include "storage/sd.h"

const uint8_t LOOP_MS = 50; // Antes uint32_t
static uint32_t lastLoop = 0;

void setup() {
    Serial.begin(115200);
    Serial.setTimeout(30);

    SDLogger::setup();
    Eeprom::setup();

    PWM::setup();

    RPM::setup();
    Dht::setup();

    Ramp::setup();

    Serial.println(F("ACK:BOOT OK"));
}

void loop() {
    // RX
    Commands::loop();
    
    uint32_t now=millis();
    if(now - lastLoop >= LOOP_MS){
        uint32_t dt_ms=now - lastLoop;
        lastLoop=now;

        // PARA FUNCIONAR TEM QUE ESTAR NESTA ORDEM!!
        Throttle::loop();

        bool idle = (Ramp::getDutyNow()<1.0f) && (RPM::getRpm() < 5.0f);
        Current::loop(idle);

        Dht::loop(now);

        float pedalPct = Throttle::getPct();
        Ramp::loop(pedalPct, now, dt_ms); 

        float dutyNowPct = Ramp::getDutyNow();
        PWM::loop(dutyNowPct);

        RPM::loop();

        Telemetry::Data telemetryData = {
            now,
            Throttle::getVolts(),
            Throttle::getPct(),
            Dht::getTemp(),
            Dht::getHumi(),
            RPM::getRpm(),
            RPM::getSpeed(),
            Current::getCurrentBattery(),
            Current::getCurrentMotor(),
            Throttle::getVoltageMin(),
            Throttle::getVoltageMax(),
            RPM::getWheelCm(),
            RPM::getPpr(),
            Ramp::getOverrideEnable(),
            Ramp::getOverridePct(),
            Ramp::getMaxPct(),
        };

        Telemetry::loop(telemetryData);
        
        SDLogger::Data sdData = {
            now,
            Throttle::getVolts(),
            Throttle::getPct(),
            Dht::getTemp(),
            Dht::getHumi(),
            RPM::getRpm(),
            RPM::getSpeed(),
            Current::getCurrentBattery(),
            Current::getCurrentMotor(),
            Ramp::getDutyNow(),
            Ramp::getDutyTarget(),
            Ramp::getMaxPct(),
        };

        SDLogger::loop(sdData);

        if (Ramp::getRampDelayMs()) 
            delay(Ramp::getRampDelayMs());
    }
}