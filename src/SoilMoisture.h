#ifndef __SOIL_MOISTURE_SENSOR_H__
#define __SOIL_MOISTURE_SENSOR_H__

#include <Arduino.h>

#define SOIL_WET 1697
#define SOIL_DRY 3650

class SoilMoisture
{
    const int SensorPin;

public:
    SoilMoisture(const int SensorPin) : SensorPin(SensorPin)
    {
    }

    int getMoisture()
    {
        int value = map(analogRead(SensorPin), SOIL_DRY, SOIL_WET, 0, 100);

        return (value < 0) ? 0 : (value > 100) ? 100 : value;
    }
};

#endif