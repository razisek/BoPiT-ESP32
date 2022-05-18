#ifndef __WIFI_CONNECT_CLIENT_CLIENT__
#define __WIFI_CONNECT_CLIENT_CLIENT__

#include <WiFi.h>

#include "GetFile.h"

// Uncomment if you want to print wifi cridentials
// #define PRINT_CREDETIALS

#define CREDENTIALS_JSON_SIZE (90)

#ifdef PRINT_CREDETIALS
#define Print(x) Serial.print(x);
#define Println(x) Serial.println(x);
#else
#define Print(x) Serial.print(x);
#define Println(x) Serial.println(x);
#endif

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
            Print("SSID => :");
            Println(ssid);
            isPasswordLess = true;
            if (doc.containsKey("password"))
            {
                password = doc["password"];
                Print("PASSWORD : ");
                Println(password);
                isPasswordLess = false;
            }
            return true;
        }
        return false;
    }

public:
    WifiConnection() : doc(CREDENTIALS_JSON_SIZE)
    {
    }

    bool getConfig()
    {
        return getAuthData(CREDENTIALS_JSON_SIZE, fileConfig.WifiClientConfig);
    }

    bool wifiConnect()
    {
        isPasswordLess ? WiFi.begin(ssid) : WiFi.begin(ssid, password);

        return (WiFi.waitForConnectResult() == WL_CONNECTED);
    }
};

#endif