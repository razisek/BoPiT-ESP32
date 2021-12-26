#ifndef __WIFI_CONNECT_CLIENT_CLIENT__
#define __WIFI_CONNECT_CLIENT_CLIENT__

#include <WiFi.h>

#include "GetFile.h"

class WifiConnection
{
    const char *ssid;
    const char *password;
    bool isPasswordLess;

    FileConfig fileConfig;
    DynamicJsonDocument doc;

    bool getAuthData(uint16_t size, uint8_t flags)
    {
        doc = fileConfig.getJson(size, flags);

        if (fileConfig.jsonIsOk() && !doc.isNull())
        {
            ssid = doc["ssid"];
            isPasswordLess = true;
            if (doc.containsKey("password"))
            {
                password = doc["password"];
                isPasswordLess = false;
            }
            return true;
        }
        return false;
    }

public:
    WifiConnection() : doc(90)
    {
    }

    bool getClientConfig()
    {
        return getAuthData(90, fileConfig.WifiClientConfig);
    }

    bool wifiConnect()
    {
        if (isPasswordLess)
        {
            WiFi.begin(ssid);
        }
        else
        {
            WiFi.begin(ssid, password);
        }
        return (WiFi.waitForConnectResult() == WL_CONNECTED);
    }
};

#endif