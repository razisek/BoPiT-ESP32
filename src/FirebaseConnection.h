#ifndef _FIRE_BASE_CONNECTIONS_
#define _FIRE_BASE_CONNECTIONS_

#define JSON_SIZE 256

#include <Arduino.h>
#include <FirebaseESP32.h>

#include "GetFile.h"

FirebaseAuth auth;
FirebaseConfig config;

class FirebaseConnection
{
    const char *host;
    const char *apiKey;
    DynamicJsonDocument doc;

public:
    FirebaseConnection() : doc(JSON_SIZE)
    {
    }

    bool getConfig()
    {
        FileConfig fileConfig;
        doc = fileConfig.getJson(JSON_SIZE, fileConfig.FirebaseConfig);

        if (fileConfig.jsonIsOk() && !doc.isNull())
        {
            host = doc["host"];
            apiKey = doc["apikey"];

            return true;
        }
        return false;
    }

    void begin()
    {
        config.database_url = host;
        config.signer.tokens.legacy_token = apiKey;

        Firebase.reconnectWiFi(true);

        Firebase.begin(&config, &auth);
    }
};

#endif