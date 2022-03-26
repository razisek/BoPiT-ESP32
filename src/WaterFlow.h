#ifndef __WATER_FLOW_H__
#define __WATER_FLOW_H__

#include <Arduino.h>

#define CALIBRATION_FACTOR (4.5)
#define WATER_FLOW_UPDATE_INTERVAL (1000)
#define WATERFLOW_SENSOR_PIN (27)

volatile byte pulseCount;

void IRAM_ATTR pulseCounter()
{
    pulseCount++;
}

class WaterFlow
{
    float flowRate;
    
    unsigned long currentMillis;
    unsigned long previousMillis;
    unsigned long totalMilliLitres;
    unsigned int flowMilliLitres;

    bool isRunning;

    byte pulse1Sec;

public:
    WaterFlow()
    {
        pinMode(WATERFLOW_SENSOR_PIN, INPUT_PULLUP);
        isRunning = false;

        pulseCount = 0;
        flowRate = 0.0;
        flowMilliLitres = 0;
        previousMillis = 0;
        currentMillis = 0;
    };

    int getTotalMilliLiters()
    {
        return totalMilliLitres;
    }

    void startReading()
    {
        previousMillis = millis();
        totalMilliLitres = 0;
        pulseCount = 0;

        isRunning = true;
        attachInterrupt(digitalPinToInterrupt(WATERFLOW_SENSOR_PIN), pulseCounter, FALLING);
    }

    void stopReading()
    {
        detachInterrupt(WATERFLOW_SENSOR_PIN);
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

            flowRate = ((1000.0 / (millis() - previousMillis)) * pulse1Sec) / CALIBRATION_FACTOR;
            previousMillis = millis();
            
            flowMilliLitres = (flowRate / 60) * 1000;
            totalMilliLitres += flowMilliLitres;
            
            Serial.println(totalMilliLitres);
        }
    }
};

#endif