#ifndef __SENSORS_H__
#define __SENSORS_H__

#include <Arduino.h>

#include "ReadDHT.h"
#include "ReadDallas.h"
#include "SoilMoisture.h"

class Sensors
{
    ReadDallas soilTempSensor;
    SoilMoisture soilMoistureSensor;
    DHTSensor dhtSensor;

public:
    Sensors(uint8_t DS18B20_PIN, uint8_t SOIL_MOISTURE_SENSOR_PIN, uint8_t DHT_SENSOR_PIN, uint8_t DHTTYPE) : soilTempSensor(DS18B20_PIN), soilMoistureSensor(SOIL_MOISTURE_SENSOR_PIN), dhtSensor(DHT_SENSOR_PIN, DHTTYPE)
    {
    }

    int getAirHumidity()
    {
        return dhtSensor.Humidity();
    }

    int getAirTemperature(){
        return dhtSensor.Temperature();
    }

    int getSoilMoisture(){
        return soilMoistureSensor.getMoisture();
    }

    int getSoilTemperature(){
        return soilTempSensor.getTemperature();
    }
};

#endif