#ifndef __READ_DHT_H__
#define __READ_DHT_H__

#include <Arduino.h>
#include "DHT.h"

#define DHTPIN 4
#define DHTTYPE DHT11

class ReadDHT
{
    DHT dht;

public:
    ReadDHT() : dht(DHTPIN, DHTTYPE)
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
        Serial.println(temp);
        return temp;
    }
};

#endif