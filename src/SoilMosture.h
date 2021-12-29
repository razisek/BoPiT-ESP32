#ifndef __SOIL_MOISTURE_SENSOR_H__
#define __SOIL_MOISTURE_SENSOR_H__

#include <Arduino.h>

class SoilMosture
{
    const int SensorPin;
    const int AirValue = 4095;
    const int WaterValue = 1270;
    // tanah kering = 40-55%
    // normal tanah semi lembab = 64%
    // tanah basah = 85%

public:
    SoilMosture(const int SensorPin) : SensorPin(SensorPin)
    {
    }

    int getKelembaban()
    {
        return map(analogRead(SensorPin), AirValue, WaterValue, 0, 100);
    }
};

#endif