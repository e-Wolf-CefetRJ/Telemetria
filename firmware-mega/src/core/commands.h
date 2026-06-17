#pragma once

#include <Arduino.h>

#include "telemetry.h"

#include "control/ramp.h"
#include "control/pwm.h"

#include "sensors/throttle/throttle.h"
#include "sensors/rpm/rpm.h"

#include "config/eeprom/eeprom.h"

namespace Commands {
    void loop();
}