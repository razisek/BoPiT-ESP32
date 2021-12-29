#ifndef __WATER_FLOW_H__
#define __WATER_FLOW_H__

#include <Arduino.h>

#define CALIBRATION_FACTOR (4.5)
#define WATER_FLOW_UPDATE_INTERVAL (1000)

volatile byte pulseCount;

class WaterFlow
{
    unsigned long currentMillis;
    unsigned long previousMillis;
    unsigned long totalMilliLitres;

    bool isRunning;

    byte pulse1Sec;

public:
    WaterFlow()
    {
        isRunning = false;

        currentMillis = 0;
        previousMillis = 0;
        pulseCount = 0;
        pulse1Sec = 0;
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
        if (isRunning && millis() - currentMillis >= WATER_FLOW_UPDATE_INTERVAL)
        {
            currentMillis = millis();

            pulse1Sec = pulseCount;
            pulseCount = 0;

            float flowRate = ((1000.0 / (millis() - previousMillis)) * pulse1Sec) / CALIBRATION_FACTOR;
            previousMillis = millis();

            totalMilliLitres += (long)(flowRate / 60) * 1000;
        }
    }
};

#endif