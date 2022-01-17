#include <Arduino.h>
#include <FirebaseESP32.h>
#include <addons/RTDBHelper.h>
#include <esp_task_wdt.h>

#include "FirebaseConnection.h"
#include "FirebaseController.h"
#include "WifiConnection.h"
#include "WaterFlow.h"
#include "Penyiram.h"

#define DELAY_UPDATE_FIREBASE (5000)
#define DELAY_GET_SERVICE (10000)
#define DELAY_DS18B20_UPDATE (2000)

#define SOIL_MOISTURE_THRESHOLD (35)
#define SOIL_MOISTURE_MINIMUM (20)
#define SOIL_MOISTURE_SENSOR_PIN (33)

#define RELAY_PIN (26)
#define WATERFLOW_SENSOR_PIN (27)

#define DHT_SENSOR_PIN (4)
#define DS18B20_PIN (32)

TaskHandle_t Task1;
TaskHandle_t Task2;

WaterFlow debit;

SoilMosture kelembaban(SOIL_MOISTURE_SENSOR_PIN);

bool onRunning = false;

int lastRun = 61;
int suhuTanah = 0;

uint8_t totalRunTime = 0;
uint8_t runTime = 0;
uint8_t SoilMosture_Treshold = SOIL_MOISTURE_THRESHOLD;
uint8_t taskOwner;

enum OwnerRun
{
  Automasi = 0,
  Jadwal,
} ownerRun;

void connect()
{
  WifiConnection KonekWifi;

  if (KonekWifi.getClientConfig())
  {
    KonekWifi.wifiConnect();
    Serial.println("connected");
  }

  FirebaseConnection fbCon;
  if (WiFi.isConnected())
  {
    fbCon.getConfig();
    fbCon.begin();
  }
}

void Task1Func(void *Parameters)
{
  Penyiram penyiram(RELAY_PIN);
  ReadDallas dallas(DS18B20_PIN);

  bool isTaskRunning = false;

  unsigned long lastMillis = millis();
  unsigned long lastMillis2 = millis();

  for (;;)
  {
    if (onRunning)
    {
      if (!isTaskRunning)
      {
        penyiram.start();
        isTaskRunning = true;
        lastMillis = millis();
      }

      if (taskOwner == Automasi)
      {
        penyiram.run(kelembaban.getKelembaban(), &isTaskRunning, SoilMosture_Treshold);
      }
      else if (taskOwner == Jadwal)
      {
        if (totalRunTime >= runTime)
        {
          penyiram.stop();
          isTaskRunning = false;
        }
      }
      debit.run();

      onRunning = isTaskRunning;
    }

    if (taskOwner == Jadwal && millis() - lastMillis >= 60000)
    {
      totalRunTime++;
      lastMillis = millis();
    }

    if (millis() - lastMillis2 >= DELAY_DS18B20_UPDATE)
    {
      suhuTanah = dallas.getSuhuTanah();
      lastMillis2 = millis();
    }

    delay(250);
  }
}

void Task2Func(void *Parameters)
{
  connect();

  FirebaseController fbData(DHT_SENSOR_PIN, DHT11, SOIL_MOISTURE_SENSOR_PIN);

  unsigned int lastmillis1 = millis();
  unsigned int lastmillis2 = millis();

  bool lastOnRunning = onRunning;

  for (;;)
  {
    if (onRunning != lastOnRunning)
    {
      Serial.println("ganti status penyiraman");

      Serial.println(debit.getTotalMilliLiters());

      debit.stopReading();
      if (debit.getTotalMilliLiters() > 0)
      {
        fbData.setSensorData(suhuTanah);
        delay(10);
        fbData.sendWaterUsageLog(debit.getTotalMilliLiters());
        delay(10);
        fbData.sendNotification(debit.getTotalMilliLiters());
      }

      lastOnRunning = onRunning;
    }

    if (millis() - lastmillis2 >= DELAY_GET_SERVICE)
    {
      if (fbData.isServiceOn())
      {
        if (!onRunning)
        {
          if (fbData.isScheduleRun(onRunning, lastRun))
          {
            runTime = fbData.jadwalRunTime();
            totalRunTime = 0;

            Serial.println("penyiraman berhasil JADWAL");

            taskOwner = Jadwal;
            onRunning = true;
            lastOnRunning = onRunning;

            lastRun = fbData.getMinute();

            debit.startReading();
          }

          if (fbData.isAutomasiRun(onRunning, suhuTanah))
          {
            SoilMosture_Treshold = fbData.getSoilThreshold();

            Serial.println("penyiraman berhasil AUTOMASI");

            taskOwner = Automasi;
            onRunning = true;
            lastOnRunning = onRunning;

            debit.startReading();
          }
        }
      }
      lastmillis2 = millis();
    }

    if (millis() - lastmillis1 >= DELAY_UPDATE_FIREBASE)
    {
      fbData.setSensorData(suhuTanah);
      lastmillis1 = millis();
    }

    if (lastRun + 1 == fbData.getMinute())
    {
      lastRun = 61;
      fbData.updateSchedule();
    }
    delay(1);
  }
}

void setup()
{
  esp_task_wdt_init(30, false);
  Serial.begin(115200);

  xTaskCreatePinnedToCore(
      Task2Func, /* Task function. */
      "Task1",   /* name of task. */
      10000,     /* Stack size of task */
      NULL,      /* parameter of the task */
      1,         /* priority of the task */
      &Task1,    /* Task handle to keep track of created task */
      0);        /* pin task to core 0 */

  xTaskCreatePinnedToCore(
      Task1Func, /* Task function. */
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