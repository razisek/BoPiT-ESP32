#pragma once
#include <Arduino.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>

#define TOTAL_FILE_CONFIG 2

class FileConfig
{
    String fileConfigName[TOTAL_FILE_CONFIG];
    DeserializationError error;

    File openConfig(uint8_t type)
    {
        return SPIFFS.open(String(fileConfigName[type]).c_str(), "r");
    }

public:
    enum configType
    {
        WifiClientConfig = 0,
        FirebaseConfig,
    };

    FileConfig()
    {
        SPIFFS.begin();
        fileConfigName[0] = "/Wifi.json";
        fileConfigName[1] = "/Firebase.json";
    }

    DynamicJsonDocument getJson(uint16_t size, uint8_t flags)
    {
        File file = openConfig(flags);
        DynamicJsonDocument doc(size);

        error = deserializeJson(doc, file);
        file.close();
        return doc;
    }

    bool jsonIsOk()
    {
        return !error;
    }
};