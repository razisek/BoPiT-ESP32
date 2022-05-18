#ifndef __FIREBASE_CONTROLLER_H__
#define __FIREBASE_CONTROLLER_H__

#include <Arduino.h>
#include <stdio.h>
#include <ArduinoJson.h>
#include <FirebaseESP32.h>
#include "addons/RTDBHelper.h"
#include "time.h"
#include "Sensors.h"

#define NTP_SERVER "pool.ntp.org"

#define DEFAULT_SOIL_MOISTURE_TRESHOLD 60

class FirebaseController
{
    FirebaseData fbdo;
    time_t now;

    struct tm *timeinfo;

    const char *ntpServer = NTP_SERVER;
    const long gmtOffset_sec = 25200;
    const int daylightOffset_sec = 0;

    uint8_t timeRunning;
    uint8_t soilMoistureMaximum;
    uint16_t scheduleTimer;

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
    FirebaseController()
    {
        configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
        timeRunning = 0;
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

    void updateSensorData(Sensors *sensors)
    {
        FirebaseJson json;

        json.add(F("AirHumidity"), sensors->getAirHumidity());
        json.add(F("AirTemperature"), sensors->getAirTemperature());
        json.add(F("SoilMoisture"), sensors->getSoilMoisture());
        json.add(F("SoilTemperature"), sensors->getSoilTemperature());

        Firebase.updateNode(fbdo, F("/SensorData"), json);
    }

    void sendNotification(int totalDebit)
    {
        String notifMessage = F("Penyiraman telah berhasil dilakukan!\nTotal air yang digunakan sebanyak ");

        Firebase.getString(fbdo, "/Token");
        fbdo.fcm.begin(F("AAAAJn7rcU0:APA91bECX7dFufaRYc9LzLbSXmIQCFpqmHiJnhg9_h-Mk2W03gaSWPeUmZTxw5235Caqcchm36tyeblsDSFLHyZEfmrA0w9gjTrWtLG6mdprG7tUw8tOUn64ak4W2Emk_ZXhKYx3EYRH"));
        fbdo.fcm.setPriority("high");
        fbdo.fcm.setTimeToLive(5000);
        fbdo.fcm.setNotifyMessage("Informasi", notifMessage + String(totalDebit) + "ml");
        fbdo.fcm.addDeviceToken(fbdo.to<String>());

        Firebase.sendMessage(fbdo, 0);
        
        fbdo.fcm.clearDeviceToken();
    }

    void sendWaterUsageLog(int totalDebit, unsigned int lastEpoch)
    {
        FirebaseJson json;
        
        json.add(F("TotalUsage"), totalDebit);
        json.add(F("Runtime"), getEpoch() - lastEpoch);

        Firebase.setJSON(fbdo, "/Log/" + String(getEpoch()), json);
    }

    bool isServiceRun()
    {
        if (Firebase.getBool(fbdo, F("/AutoSprinkler")))
        {
            return fbdo.to<bool>();
        }

        return false;
    }

    int getSoilMoistureThreshold()
    {
        return soilMoistureMaximum;
    }

    int scheduleRunTime()
    {
        return scheduleTimer;
    }

    bool isScheduleRun(bool isRunning, int lastTime)
    {
        DynamicJsonDocument doc(96);
        Firebase.get(fbdo, "/Schedule/" + String(getHour()));

        if (fbdo.jsonString() != NULL)
        {
            deserializeJson(doc, fbdo.jsonString());
            bool enable = doc["Enable"];
            timeRunning = (int) doc["Minute"];
            scheduleTimer = doc.containsKey("Runtime") ? doc["Runtime"] : 1;

            if (enable && timeRunning == getMinute() && !isRunning && timeRunning != lastTime)
            {
                return true;
            }
        }

        return false;
    }

    void updateSchedule()
    {
        timeRunning = 61;
    }

    bool soilAutomationCheck(bool isRunning, int soilMoisture)
    {
        DynamicJsonDocument doc(256);

        Firebase.get(fbdo, F("/Automation"));
        deserializeJson(doc, fbdo.jsonString());

        if (doc.containsKey("Moisture") && !isRunning)
        {
            JsonObject jsonData = doc["Moisture"];

            bool isEnabled = jsonData["Enable"];
            const char *getOprs = jsonData["Operator"];
            int soilMoistureMinimum = jsonData["Value"];

            soilMoistureMaximum = jsonData.containsKey("Threshold") ? jsonData["Threshold"] : DEFAULT_SOIL_MOISTURE_TRESHOLD;

            if (isEnabled)
            {
                if (*getOprs == '<')
                {
                    return soilMoisture < soilMoistureMinimum;
                }
                else if (*getOprs == '=')
                {
                    return soilMoisture == soilMoistureMinimum;
                }
                else if (*getOprs == '>')
                {
                    return soilMoisture > soilMoistureMinimum;
                }
            }
        }
        return false;
    }
};

#endif