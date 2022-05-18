#ifndef __READ_DHT_H__
#define __READ_DHT_H__

#include <Arduino.h>
#include "DHT.h"

class DHTSensor
{
    DHT dht;

    uint8_t lastHum;
    int lastTemp;

public:
    DHTSensor(int dhtPin, int dhtType) : dht(dhtPin, dhtType)
    {
        dht.begin();

        lastHum = 0;
        lastTemp = 0;
    }

    int Humidity()
    {
        float humidity = dht.readHumidity();
        if (!isnan(humidity))
        {
            lastHum = humidity;
            return humidity;
        }
        else
        {
            return lastHum;
        }
    }

    int Temperature()
    {
        float temp = dht.readTemperature();
        if (!isnan(temp))
        {
            lastTemp = temp;
            return temp;
        }
        else
        {
            return lastTemp;
        }
    }
};

#endif