#ifndef __READ_DHT_H__
#define __READ_DHT_H__

#include <Arduino.h>
#include "DHT.h"

class ReadDHT
{
    DHT dht;

public:
    ReadDHT(int dhtPin, int dhtType) : dht(dhtPin, dhtType)
    {
        dht.begin();
    }

    int KelembabanUdara()
    {
        float humidity = dht.readHumidity();
        return humidity;
    }

    int SuhuUdara()
    {
        float temp = dht.readTemperature();
        return temp;
    }
};

#endif