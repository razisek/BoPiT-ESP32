#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

const int oneWireBus = 25;
OneWire oneWire(oneWireBus);
DallasTemperature sensors(&oneWire);

class ReadDallas
{

public:
    ReadDallas()
    {
        sensors.begin();
    }

    int getSuhuTanah()
    {
        sensors.requestTemperatures();
        float temperatureC = sensors.getTempCByIndex(0);
        return temperatureC;
    }
};