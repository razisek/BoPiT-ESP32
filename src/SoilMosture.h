#ifndef __SOIL_MOISTURE_SENSOR_H__
#define __SOIL_MOISTURE_SENSOR_H__

#include <Arduino.h>

#define SOIL_WET 1697
#define SOIL_DRY 3650

class SoilMosture
{
    const int SensorPin;
    // tanah kering = 40-55%
    // normal tanah semi lembab = 64%
    // tanah basah = 85%

public:
    SoilMosture(const int SensorPin) : SensorPin(SensorPin)
    {
    }

    int getKelembaban()
    {
        int value = map(analogRead(SensorPin), SOIL_DRY, SOIL_WET, 0, 100);
        if (value < 0)
        {
            value = 0;
        }
        else if (value > 100)
        {
            value = 100;
        }
        return value;
    }
};

#endif