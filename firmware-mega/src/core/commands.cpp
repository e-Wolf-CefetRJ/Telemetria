#include "commands.h"

namespace {
    typedef void (*CommandHandler)(const char* args);

    struct Command {
        const char* name;
        CommandHandler handler;
    };

    // Auxiliar
    void ack(const char* k, const String& v){
        Serial.print(F("ACK:")); 
        Serial.print(k); 
        Serial.print(' '); 
        Serial.println(v);
    }

    // Handler
    namespace {
        void start(const char*) {
            Ramp::start();
            ack("START", "");
        }

        void stop(const char*) {
            Ramp::stop();
            ack("STOP", "");
        }

        void hold(const char* args) {
            int pct = atoi(args);
            pct = constrain(pct, 0, 100);
            // Se der erro, tirar esse buffer
            char buf[4];
            itoa(pct, buf, 10);
            Ramp::hold(pct);
            ack("HOLD", buf);
        }

        void setMin(const char*) {
            Throttle::setVoltageMin(Throttle::getVolts());
            char buf[8];
            dtostrf(Throttle::getVoltageMin(), 0, 3, buf);
            ack("SET_MIN_NOW", buf);
        }

        void setMax(const char*) {
            Throttle::setVoltageMax(Throttle::getVolts());
            char buf[8];
            dtostrf(Throttle::getVoltageMax(), 0, 3, buf);
            ack("SET_MAX_NOW", buf);
        }

        void defaults(const char*) {
            Eeprom::reset();
            PWM::setFrequency(PWM::getFrequency());
            ack("DEFAULTS", "OK");
        }

        void setWheel(const char* args) {
            float cm = atof(args);
            cm = constrain(cm, 0.5f, 200.0f);
            RPM::setWheelCm(cm);
            char buf[8];
            dtostrf(cm, 0, 1, buf);
            ack("SET_WHEEL", buf);
        }

        void setPpr(const char* args) {
            int n = atoi(args);
            n = constrain(n, 1, 16);
            RPM::setPpr((uint8_t)n);
            char buf[4];
            itoa(n, buf, 10);
            ack("SET_PPR", buf);
        }

        void setPwmf(const char* args) {
            uint16_t hz = (uint16_t)atoi(args);
            PWM::setFrequency(hz);
            char buf[6];
            itoa(hz, buf, 10);
            ack("SET_PWMF", buf);
        }

        void setStartMin(const char* args) {
            uint8_t x = (uint8_t)atoi(args);
            Ramp::setStartMin(constrain(x, 0, 40));
            char buf[4];
            itoa(Ramp::getStartMin(), buf, 10);
            ack("SET_STARTMIN", buf);
        }

        void setRapidMs(const char* args) {
            uint16_t ms = (uint16_t)atoi(args);
            Ramp::setRapidRamp(constrain(ms, 50, 1500));
            char buf[6];
            itoa(Ramp::getRapidRamp(), buf, 10);
            ack("SET_RAPIDMS", buf);
        }

        void setRapidup(const char* args) {
            float up = atof(args);
            Ramp::setRapidUp(constrain(up, 10.0f, 400.0f));
            char buf[8];
            dtostrf(Ramp::getRapidUp(), 0, 1, buf);
            ack("SET_RAPIDUP", buf);
        }

        void setSlew(const char* args) {
            // Copia args pois strtok modifica a string
            char argscopy[32];
            strncpy(argscopy, args, sizeof(argscopy) - 1);
            argscopy[sizeof(argscopy) - 1] = '\0';
            
            char* token = strtok(argscopy, " ");
            if (token == nullptr) return;
            float up = atof(token);
            
            token = strtok(nullptr, " ");
            if (token == nullptr) return;
            float dn = atof(token);
            
            Ramp::setSlewUp(constrain(up, 5.0f, 200.0f));
            Ramp::setSlewDown(constrain(dn, 5.0f, 300.0f));
            
            char buf[32];
            char upbuf[8], dnbuf[8];
            dtostrf(Ramp::getSlewUp(), 0, 1, upbuf);
            dtostrf(Ramp::getSlewDown(), 0, 1, dnbuf);
            snprintf(buf, sizeof(buf), "%s,%s", upbuf, dnbuf);
            ack("SET_SLEW", buf);
        }

        void setZeroTo(const char* args) {
            unsigned long us = strtoul(args, nullptr, 10);
            RPM::setZeroTimeout(us);
            char buf[12];
            ultoa(us, buf, 10);
            ack("SET_ZEROTO", buf);
        }

        void save(const char*) {
            Eeprom::save();
            ack("SAVE", "OK");
        }

        void loadDefaults(const char*) {
            Eeprom::reset();
            PWM::setFrequency(PWM::getFrequency());
            ack("LOAD_DEFAULTS", "OK");
        }

        void setMinV(const char* args) {
            Throttle::setVoltageMin(atof(args));
            char buf[8];
            dtostrf(Throttle::getVoltageMin(), 0, 3, buf);
            ack("SET_MINV", buf);
        }

        void setMaxV(const char* args) {
            Throttle::setVoltageMax(atof(args));
            char buf[8];
            dtostrf(Throttle::getVoltageMax(), 0, 3, buf);
            ack("SET_MAXV", buf);
        }

        void setMaxPct(const char* args) {
            int x = atoi(args);
            x = constrain(x, 1, 100);
            Ramp::setMaxPct((uint8_t) x );
            char buf[4];
            itoa(Ramp::getMaxPct(), buf, 10);
            ack("SET_MAXPCT", buf);
        }

        void setPrintmode(const char* args) {
            // args pode ser "KVP" ou "PLOTTER"
            if (strcasecmp(args, "KVP") == 0) {
                Telemetry::setPrintMode(0);
            } else if (strcasecmp(args, "PLOTTER") == 0) {
                Telemetry::setPrintMode(1);
            }
            char buf[2];
            itoa(Telemetry::getPrintMode(), buf, 10);
            ack("SET_PRINTMODE", buf);
        }

        void setStepMode(const char* args) {
            int x = atoi(args);
            Ramp::setStepMode(x != 0);
            ack("SET_STEP_MODE", Ramp::getStepMode() ? "1" : "0");
        }

        void setAccelsOn(const char* args) {
            int x = atoi(args);
            Ramp::setAccelSmoothing(x != 0);
            ack("SET_ACCELS_ON", Ramp::getAccelSmoothing() ? "1" : "0");
        }

        void setRampDelay(const char* args) {
            int ms = atoi(args);
            Ramp::setRampDelayMs((uint16_t)constrain(ms, 0, 10));
            char buf[4];
            itoa(Ramp::getRampDelayMs(), buf, 10);
            ack("SET_RAMPDELAY", buf);
        }

        void setRpmMinPulse(const char* args) {
            unsigned long us = strtoul(args, nullptr, 10);
            RPM::setMinPulse(us);
            char buf[12];
            ultoa(us, buf, 10);
            ack("SET_RPM_MINPULSE", buf);
        }
    }

    const Command commandTable[] = {
        // Controle da rampa
        {"START",            start},
        {"STOP",             stop},
        {"HOLD",             hold},
        {"SET_STARTMIN",     setStartMin},
        {"SET_RAPIDMS",      setRapidMs},
        {"SET_RAPIDUP",      setRapidup},
        {"SET_SLEW",         setSlew},
        {"SET_MAXPCT",       setMaxPct},
        {"SET_STEP_MODE",    setStepMode},
        {"SET_ACCELS_ON",    setAccelsOn},
        {"SET_RAMPDELAY",    setRampDelay},

        // Throttle
        {"SET_MIN_NOW",      setMin},
        {"SET_MAX_NOW",      setMax},
        {"SET_MINV",         setMinV},
        {"SET_MAXV",         setMaxV},

        // RPM
        {"SET_WHEEL",        setWheel},
        {"SET_PPR",          setPpr},
        {"SET_ZEROTO",       setZeroTo},
        {"SET_RPM_MINPULSE", setRpmMinPulse},

        // PWM
        {"SET_PWMF",         setPwmf},

        // EEPROM
        {"SAVE",             save},
        {"DEFAULTS",         defaults},
        {"LOAD_DEFAULTS",    loadDefaults},

        // Telemetria
        {"SET_PRINTMODE",    setPrintmode}
    };

    const uint8_t commandCount = sizeof(commandTable) / sizeof(commandTable[0]);

}


namespace Commands {
    void processLine(char* line) {
        line[strcspn(line, "\r\n")] = '\0';
        
        // Pular espaços iniciais
        while (*line == ' ') line++;
        if (*line == '\0') return;
        
        // Encontrar o primeiro espaço (separador comando/argumentos)
        char* args = strchr(line, ' ');
        if (args != nullptr) {
            *args = '\0';
            args++;
            // Pular espaços nos argumentos
            while (*args == ' ') args++;
        }
        
        // Buscar comando na tabela
        for (uint8_t i = 0; i < commandCount; i++) {
            if (strcasecmp(line, commandTable[i].name) == 0) {
                commandTable[i].handler(args ? args : "");
                return;
            }
        }
        
        // Comando desconhecido
        Serial.print(F("ERR: unknown command '"));
        Serial.print(line);
        Serial.println(F("'"));
    }

    void loop() {
        static char rxBuffer[128];
        static uint8_t rxIndex = 0;
        
        while (Serial.available()) {
            char c = Serial.read();
            if (c == '\n') {
                rxBuffer[rxIndex] = '\0';
                processLine(rxBuffer);
                rxIndex = 0;    
            } else if (c != '\r' && rxIndex < sizeof(rxBuffer) - 1) {
                rxBuffer[rxIndex++] = c;
            }
        }
    }
}