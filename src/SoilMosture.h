#include <Arduino.h>

class SoilMosture
{
    const int SensorPin = 35;
    int soilMoistureValue = 0;
    int soilmoisturepercent = 0;
    const int AirValue = 4095;
    const int WaterValue = 1270;
    //tanah kering = 40-55%
    //normal tanah semi lembab = 64%
    //tanah basah = 85%

public:
    SoilMosture()
    {
    }

    int getKelembaban()
    {
        soilMoistureValue = analogRead(SensorPin);
        soilmoisturepercent = map(soilMoistureValue, AirValue, WaterValue, 0, 100);
        return soilmoisturepercent;
    }
};