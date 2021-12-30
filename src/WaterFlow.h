#ifndef __WATER_FLOW_H__
#define __WATER_FLOW_H__

#include <Arduino.h>

#define CALIBRATION_FACTOR (4.5)
#define WATER_FLOW_UPDATE_INTERVAL (1000)

volatile byte pulseCount;
float calibrationFactor = 4.5;

class WaterFlow
{
    long currentMillis = 0;
    long previousMillis = 0;
    unsigned long totalMilliLitres;
    float flowRate;
    unsigned int flowMilliLitres;

    bool isRunning;

    byte pulse1Sec;

public:
    WaterFlow()
    {
        isRunning = false;

        pulseCount = 0;
        flowRate = 0.0;
        flowMilliLitres = 0;
        previousMillis = 0;
    };

    int getTotalMilliLiters()
    {
        return totalMilliLitres;
    }

    void startReading()
    {
        previousMillis = millis();
        totalMilliLitres = 0;

        isRunning = true;
    }

    void stopReading()
    {
        pulseCount = 0;

        isRunning = false;
    }

    void run()
    {
        currentMillis = millis();
        if (isRunning && currentMillis - previousMillis > WATER_FLOW_UPDATE_INTERVAL)
        {
            pulse1Sec = pulseCount;
            pulseCount = 0;
            flowRate = ((1000.0 / (millis() - previousMillis)) * pulse1Sec) / calibrationFactor;
            previousMillis = millis();
            flowMilliLitres = (flowRate / 60) * 1000;
            totalMilliLitres += flowMilliLitres;
            Serial.println(totalMilliLitres);
        }
    }
};

#endif