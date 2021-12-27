#ifndef __PENYIRAM_H__
#define __PENYIRAM_H__

#include <Arduino.h>

#define DEFAULT_RELAY_STATUS LOW


class Penyiram
{
    const int *relayPin = 0;
    const int *threshold = 0;
    bool pumpStatus;

public:
    Penyiram(int relayPin, int threshold)
    {
        Penyiram::relayPin = &relayPin;
        Penyiram::threshold = &threshold;
        pumpStatus = false;

        pinMode(relayPin, OUTPUT);

        digitalWrite(relayPin, DEFAULT_RELAY_STATUS);
    }

    void start()
    {
        digitalWrite(*relayPin, HIGH);
        pumpStatus = true;
    }

    void run(int soilValue, bool *onRunning){
        if (pumpStatus && soilValue >= *threshold){
            digitalWrite(*relayPin, LOW);
            pumpStatus = false;
            *onRunning = false;
        }
    }
};

#endif