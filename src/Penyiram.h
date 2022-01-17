#ifndef __PENYIRAM_H__
#define __PENYIRAM_H__

#include <Arduino.h>

#define DEFAULT_RELAY_STATUS HIGH

#if DEFAULT_RELAY_STATUS == HIGH
#define RELAY_ON LOW
#define RELAY_OFF HIGH
#else
#define RELAY_ON HIGH
#define RELAY_OFF LOW
#endif

class Penyiram
{
    const uint8_t relayPin;

    bool pumpStatus;

public:
    Penyiram(const int relayPin) : relayPin(relayPin)
    {
        pumpStatus = false;

        pinMode(relayPin, OUTPUT);

        digitalWrite(relayPin, RELAY_OFF);
    }

    void start()
    {
        digitalWrite(relayPin, RELAY_ON);
        pumpStatus = true;
    }

    void stop()
    {
        digitalWrite(relayPin, RELAY_OFF);
        pumpStatus = false;
    }

    void run(int soilValue, bool *onRunning, int threshold)
    {
        if (pumpStatus && soilValue >= threshold)
        {
            Serial.println("mandek");

            digitalWrite(relayPin, RELAY_OFF);

            pumpStatus = false;
            *onRunning = false;
        }
    }
};

#endif