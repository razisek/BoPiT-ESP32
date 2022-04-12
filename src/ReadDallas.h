#ifndef __READ_DALLAS_H__
#define __READ_DALLAS_H__

#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

class ReadDallas
{
    OneWire oneWire;
    DallasTemperature sensors;

public:
    ReadDallas(const int oneWireBus) : oneWire(oneWireBus), sensors(&oneWire)
    {
        sensors.begin();
    }

    int getTemperature()
    {
        sensors.requestTemperatures();
        delay(3);
        return (float) sensors.getTempCByIndex(0);
    }
};

#endif