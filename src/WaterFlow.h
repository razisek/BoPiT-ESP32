#ifndef __WATER_FLOW_H__
#define __WATER_FLOW_H__

#include <Arduino.h>

long currentMillis = 0;
long previousMillis = 0;
byte pulse1Sec = 0;
float calibrationFactor = 4.5;
int interval = 1000;
volatile byte pulseCount;
float flowRate;
unsigned int flowMilliLitres;
unsigned long totalMilliLitres;

class WaterFlow
{
public:
    WaterFlow()
    {
        pulseCount = 0;
        flowRate = 0.0;
        flowMilliLitres = 0;
        previousMillis = 0;
    };

    int getWaterFlow(bool stop = false)
    {
        Serial.println(totalMilliLitres);
        currentMillis = millis();
        if (stop)
        {
            totalMilliLitres = 0;
        }
        if (currentMillis - previousMillis > interval && !stop)
        {
            pulse1Sec = pulseCount;
            pulseCount = 0;
            flowRate = ((1000.0 / (millis() - previousMillis)) * pulse1Sec) / calibrationFactor;
            previousMillis = millis();
            flowMilliLitres = (flowRate / 60) * 1000;
            totalMilliLitres += flowMilliLitres;
            return totalMilliLitres;
        }
    }
};

#endif