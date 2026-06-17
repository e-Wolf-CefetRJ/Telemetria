#include "sd.h"

namespace {
    // RTC (Real Time Clock)
    ThreeWire rtcWire(Pins::RTC_DATA, Pins::RTC_CLOCK, Pins::RTC_RESET);
    RtcDS1302<ThreeWire> rtc(rtcWire);
    bool rtcFine = false;

    void printTimestamp(File &f) {
        if (rtcFine) {
            RtcDateTime now = rtc.GetDateTime();
            char buf[20];

            // yyyy-mm-dd HH:MM:SS
            snprintf(buf, sizeof(buf),
            "%04u-%02u-%02u %02u:%02u:%02u",
            now.Year(), now.Month(), now.Day(),
            now.Hour(), now.Minute(), now.Second()
            );
            f.print(buf);
        } else {
            f.print(millis());
        }
    }

    // SD
    File logFile;
    bool sdFine = false;
    unsigned long lastFlushMs = 0;
    
    bool createNewFile(){
        char filename[16];
        for (uint16_t i = 0; i < 1000; i++) {
            snprintf(filename, sizeof(filename), "LOG%03u.CSV", i);

            if (!SD.exists(filename)) {
                logFile = SD.open(filename, FILE_WRITE);

                if (logFile) {
                    // cabeçalho
                    logFile.println(F("ts_iso,ms,volts,pct,temp,humi,rpm,speed_kmh,Ibat,Imot,dutyNow,dutyTarget,maxpct"));
                    logFile.flush();
                    Serial.print(F("ACK:SD_FILE "));
                    Serial.println(filename);
                    return true;
                } else {
                    return false;
                }
            }
        }
        return false;
    }
}

namespace SDLogger {
    void logLine(const Data& data) {
        if (!sdFine || !logFile) return;

        // monta linha CSV
        printTimestamp(logFile);            logFile.print(',');
        logFile.print(millis());            logFile.print(',');
        logFile.print(data.volts,3);           logFile.print(',');
        logFile.print(data.pct,1);             logFile.print(',');
        if (isnan(data.temp)) logFile.print(""); else logFile.print(data.temp,1);
        logFile.print(',');
        if (isnan(data.humi)) logFile.print(""); else logFile.print(data.humi,1);
        logFile.print(',');
        logFile.print(data.rpm,1);             logFile.print(',');
        logFile.print(data.speedKmh,2);       logFile.print(',');
        logFile.print(data.currentBat,2);   logFile.print(',');
        logFile.print(data.currentMot,2);   logFile.print(',');
        logFile.print(data.dutyNowPct,1);        logFile.print(',');
        logFile.print(data.dutyTargetPct,1);     logFile.print(',');
        logFile.print((int)data.maxPct);
        logFile.println();

        // flush suave
        unsigned long now = millis();
        if (now - lastFlushMs >= 1000) {
            logFile.flush();
            lastFlushMs = now;
        }
    }

    void setup() {
        // RTC
        rtc.Begin();

        if (!rtc.GetIsRunning()) {
            rtc.SetIsRunning(true);
            delay(100);
        }

        // Verifica se a data/hora é válida
        if (!rtc.IsDateTimeValid()) {
            // sincroniza automaticamente com horário de compilação
            RtcDateTime compileTime(__DATE__, __TIME__);
            rtc.SetDateTime(compileTime);
        }

        rtcFine =  rtc.GetIsRunning();

        // SD
        pinMode(Pins::SPI_SS, OUTPUT); // SS do MEGA deve ser OUTPUT
        if (SD.begin(Pins::SD_CS)) {
            if (createNewFile()) {
                sdFine = true;
                Serial.println(F("ACK:SD OK"));
            } else {
                Serial.println(F("ERR:SD NOFILE"));
                sdFine = false;
            }
        } else {
            Serial.println(F("ERR:SD INIT"));
            sdFine = false;
        }
    }


    void loop(const Data& data) {
        logLine(data);
    }
}