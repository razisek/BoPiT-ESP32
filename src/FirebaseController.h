#ifndef __FIREBASE_CONTROLLER_H__
#define __FIREBASE_CONTROLLER_H__

#include <Arduino.h>
#include <stdio.h>
#include <ArduinoJson.h>
#include <FirebaseESP32.h>
#include "addons/RTDBHelper.h"
#include "time.h"
#include <ReadDHT.h>
#include <SoilMosture.h>
#include <ReadDallas.h>

#define NTP_SERVER "pool.ntp.org"

class FirebaseController
{
    FirebaseData fbdo;
    SoilMosture lembab;
    ReadDHT dht;
    time_t now;

    struct tm *timeinfo;

    const char *ntpServer = NTP_SERVER;
    const long gmtOffset_sec = 25200;
    const int daylightOffset_sec = 0;

    uint8_t lastRun;
    uint8_t soilMoistureThreshold;
    uint16_t jadwalTimer;

    void showError()
    {
        Serial.println(fbdo.errorReason());
    }

    unsigned long getHour()
    {
        time(&now);
        timeinfo = localtime(&now);
        return timeinfo->tm_hour;
    }

public:
    FirebaseController(int dhtPin, int dhtType, int soilMoisturePin) : lembab(soilMoisturePin), dht(dhtPin, dhtType)
    {
        configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
        lastRun = 0;
    }

    unsigned long getEpoch()
    {
        time_t now;
        struct tm timeinfo;
        if (!getLocalTime(&timeinfo))
        {
            return (0);
        }
        time(&now);
        return now;
    }

    unsigned long getMinute()
    {
        time(&now);
        timeinfo = localtime(&now);

        return timeinfo->tm_min;
    }

    void setSensorData(int suhuTanah)
    {
        FirebaseJson json;

        json.add("AirHumidity", dht.KelembabanUdara());
        json.add("AirTemperature", dht.SuhuUdara());
        json.add("SoilMoisture", lembab.getKelembaban());
        json.add("SoilTemperature", suhuTanah);
        Firebase.updateNode(fbdo, F("/SensorData"), json);
    }

    void sendNotification(int totalDebit)
    {
        Firebase.getString(fbdo, "/Token");
        fbdo.fcm.begin(F("AAAAJn7rcU0:APA91bECX7dFufaRYc9LzLbSXmIQCFpqmHiJnhg9_h-Mk2W03gaSWPeUmZTxw5235Caqcchm36tyeblsDSFLHyZEfmrA0w9gjTrWtLG6mdprG7tUw8tOUn64ak4W2Emk_ZXhKYx3EYRH"));
        fbdo.fcm.setPriority("high");
        fbdo.fcm.setTimeToLive(5000);
        fbdo.fcm.setNotifyMessage("Informasi", "Penyiraman telah berhasil dilakukan!\nTotal air yang digunakan sebanyak " + String(totalDebit) + "ml");
        fbdo.fcm.setDataMessage("{\"debit\":" + String(totalDebit) + "}");
        fbdo.fcm.addDeviceToken(fbdo.to<String>());
        Firebase.sendMessage(fbdo, 0);
        fbdo.fcm.clearDeviceToken();
    }

    void sendWaterUsageLog(int totalDebit, unsigned int lastEpoch)
    {
        FirebaseJson json;
        json.add("TotalUsage", totalDebit);
        json.add("Runtime", getEpoch() - lastEpoch);
        Firebase.setJSON(fbdo, "/Log/" + String(getEpoch()), json);
    }

    bool isServiceOn()
    {
        if (Firebase.getBool(fbdo, F("/AutoSprinkler")))
        {
            return fbdo.to<bool>();
        }

        return false;
    }

    int getSoilThreshold()
    {
        return soilMoistureThreshold;
    }

    int jadwalRunTime()
    {
        return jadwalTimer;
    }

    bool isScheduleRun(bool running, int last)
    {
        Serial.println(getHour());
        DynamicJsonDocument doc(96);
        Firebase.get(fbdo, "/Schedule/" + String(getHour()));

        if (fbdo.jsonString() != NULL)
        {
            deserializeJson(doc, fbdo.jsonString());
            bool enable = doc["Enable"];
            int minute = doc["Minute"];
            lastRun = minute;
            jadwalTimer = doc.containsKey("Runtime") ? doc["Runtime"] : 1;

            if (enable && minute == getMinute() && !running && lastRun != last)
            {
                return true;
            }
        }

        return false;
    }

    void updateSchedule()
    {
        lastRun = 61;
    }

    bool isAutomasiRun(bool running, int suhuTanah)
    {
        DynamicJsonDocument doc(256);

        Firebase.get(fbdo, F("/Automation"));
        deserializeJson(doc, fbdo.jsonString());

        if (doc.containsKey("Moisture") && !running)
        {
            JsonObject kelembaban = doc["Moisture"];
            bool isEnable = kelembaban["Enable"];
            const char *getOprs = kelembaban["Operator"];
            int value = kelembaban["Value"];
            soilMoistureThreshold = kelembaban.containsKey("Threshold") ? kelembaban["Threshold"] : 60;
            
            if (isEnable)
            {
                if (*getOprs == '<')
                {
                    return lembab.getKelembaban() < value;
                }
                else if (*getOprs == '=')
                {
                    return lembab.getKelembaban() == value;
                }
                else if (*getOprs == '>')
                {
                    return lembab.getKelembaban() > value;
                }
            }
        }
        // else if (doc.containsKey("Temperature") && !running)
        // {
        //     JsonObject Suhu = doc["Temperature"];
        //     bool isEnable = Suhu["Enable"];
        //     const char *getOprs = Suhu["Operator"];
        //     int value = Suhu["Value"];
        //     soilMoistureThreshold = Suhu.containsKey("Threshold") ? Suhu["Threshold"] : 30;

        //     if (isEnable)
        //     {
        //         if (*getOprs == '<')
        //         {
        //             return suhuTanah < value;
        //         }
        //         else if (*getOprs == '=')
        //         {
        //             return suhuTanah == value;
        //         }
        //         else if (*getOprs == '>')
        //         {
        //             return suhuTanah > value;
        //         }
        //     }
        // }

        return false;
    }
};

#endif