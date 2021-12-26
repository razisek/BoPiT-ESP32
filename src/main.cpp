#include <Arduino.h>
#include <WifiConnection.h>
#include <FirebaseESP32.h>
#include <FirebaseController.h>
#include <FirebaseConnection.h>
#include <addons/RTDBHelper.h>

#define DELAY_UPDATE_FIREBASE (600000) //10 menit

bool wifiStatus = 0, onRunning = 0;
int nowMinute = 0;
unsigned int lastmillis = millis();

void setup()
{
  Serial.begin(115200);
}

void loop()
{
  FirebaseData fbdo;
  WifiConnection KonekWifi;
  SoilMosture kelembaban;
  if (KonekWifi.getClientConfig() && !wifiStatus)
  {
    wifiStatus = 1;
    KonekWifi.wifiConnect();
    Serial.println("konekted");
  }
  FirebaseConnection fbCon;
  fbCon.getConfig();
  fbCon.begin();

  FirebaseController fbData;
  if (fbData.isServiceOn())
  {
    if (nowMinute + 1 <= fbData.getMinute() && onRunning)
    {
      Serial.println("Ganti Status Penyiraman");
      onRunning = 0;
    }

    if (fbData.isScheduleRun(onRunning))
    {
      Serial.println("PENYIRAMAN BERHASIL");
      nowMinute = fbData.getMinute();
      onRunning = 1;
    }

    if (fbData.isAutomasiRun(onRunning))
    {
      Serial.println("PENYIRAMAN BERHASIL");
      nowMinute = fbData.getMinute();
      onRunning = 1;
    }
  }

  if (millis() - lastmillis >= DELAY_UPDATE_FIREBASE)
  {
    fbData.setSensorData();
    lastmillis = millis();
  }

  delay(1000);
}