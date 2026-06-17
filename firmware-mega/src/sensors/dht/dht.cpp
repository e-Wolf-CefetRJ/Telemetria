#include "dht.h"
#define DHTTYPE DHT22

namespace {
    DHT dht(Pins::DHT22, DHTTYPE);

    struct Data {
        float temp = NAN;
        float humi = NAN;
    };
    Data data;
    
    uint32_t lastDHT = 0;
    constexpr uint32_t DHT_MS = 2000;


    static void setData(uint32_t now) {
        if(now - lastDHT >= DHT_MS) {
            lastDHT = now;

            const float temp = dht.readTemperature();
            const float humi = dht.readHumidity();

            if(!isnan(temp))
                data.temp = temp;

            if(!isnan(humi))
                data.humi = humi;
        }
    }
}

namespace Dht {
    void setup() {
        dht.begin();
    }

    void loop(uint32_t now) {
        setData(now);
    }

    float getTemp() {
        return data.temp;
    }

    float getHumi() {
        return data.humi;
    }
}