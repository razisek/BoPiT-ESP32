#ifndef __READ_DHT_H__
#define __READ_DHT_H__

#include <Arduino.h>
#include "DHT.h"

class ReadDHT
{
    DHT dht;

    uint8_t lastHum;
    int lastTemp;

public:
    ReadDHT(int dhtPin, int dhtType) : dht(dhtPin, dhtType)
    {
        dht.begin();

        lastHum = 0;
        lastTemp = 0;
    }

    int KelembabanUdara()
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

    int SuhuUdara()
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