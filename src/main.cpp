#include <Arduino.h>
#include <WifiConnection.h>
#include <FirebaseESP32.h>
#include <FirebaseController.h>
#include <FirebaseConnection.h>
#include <addons/RTDBHelper.h>

#include "Penyiram.h"

#define DELAY_UPDATE_FIREBASE (600000) // 10 menit
#define SOIL_MOISTURE_THRESHOLD (64)
#define SOIL_MOISTURE_SENSOR_PIN (35)


bool wifiStatus = false, onRunning = false;
int nowMinute = 0;
unsigned int lastmillis = millis();

TaskHandle_t Task1;
TaskHandle_t Task2;

void Task1Func(void *Parameters)
{
  FirebaseData fbdo;
  WifiConnection KonekWifi;

  if (KonekWifi.getClientConfig() && !wifiStatus)
  {
    wifiStatus = 1;
    KonekWifi.wifiConnect();
    Serial.println("konekted");
  }

  FirebaseConnection fbCon;
  if (WiFi.isConnected())
  {
    fbCon.getConfig();
    fbCon.begin();
  }
  FirebaseController fbData(25, 4, DHT11, SOIL_MOISTURE_SENSOR_PIN);

  for (;;)
  {
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
    delay(5);
  }
}

void Task2Func(void *Parameters)
{
  bool isTaskRunning = false;

  Penyiram penyiram(13, SOIL_MOISTURE_THRESHOLD);
  SoilMosture kelembaban(SOIL_MOISTURE_SENSOR_PIN);

  for (;;)
  {
    if (onRunning){
      if (!isTaskRunning){
        penyiram.start();
        isTaskRunning = true;
      }

      penyiram.run(kelembaban.getKelembaban(), &isTaskRunning);
    }

    delay(5);
  }
}

void setup()
{
  Serial.begin(115200);

  xTaskCreatePinnedToCore(
      Task1Func, /* Task function. */
      "Task1",   /* name of task. */
      10000,     /* Stack size of task */
      NULL,      /* parameter of the task */
      1,         /* priority of the task */
      &Task1,    /* Task handle to keep track of created task */
      0);        /* pin task to core 0 */

  xTaskCreatePinnedToCore(
      Task2Func, /* Task function. */
      "Task2",   /* name of task. */
      10000,     /* Stack size of task */
      NULL,      /* parameter of the task */
      1,         /* priority of the task */
      &Task2,    /* Task handle to keep track of created task */
      1);        /* pin task to core 1 */
}

void loop()
{
}