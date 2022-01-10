#ifndef __PENYIRAM_H__
#define __PENYIRAM_H__

#include <Arduino.h>

#define DEFAULT_RELAY_STATUS HIGH

class Penyiram
{
    const int relayPin;
    const int threshold;
    
    bool pumpStatus;

public:
    Penyiram(const int relayPin, const int threshold) : relayPin(relayPin), threshold(threshold)
    {
        pumpStatus = false;

        pinMode(relayPin, OUTPUT);

        digitalWrite(relayPin, DEFAULT_RELAY_STATUS);
    }

    void start()
    {
        digitalWrite(relayPin, LOW);
        pumpStatus = true;
    }

    void run(int soilValue, bool *onRunning){
        if (pumpStatus && soilValue >= threshold){
            Serial.println("mandek");

            digitalWrite(relayPin, HIGH);

            pumpStatus = false;
            *onRunning = false;
        }
    }
};

#endif