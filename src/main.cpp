#include <Arduino.h>
#include <WifiConnection.h>
#include <FirebaseESP32.h>
#include <FirebaseController.h>
#include <FirebaseConnection.h>
#include <addons/RTDBHelper.h>
#include <WaterFlow.h>
#include "Penyiram.h"

// pin 27 = waterflow
// pin 33 = soil mosture
// pin 4 = DHT
// pin 32 = dallas

// #define DELAY_UPDATE_FIREBASE (600000) // 10 menit
#define DELAY_UPDATE_FIREBASE (1000) // 10 menit
#define DELAY_GET_SERVICE (10000)      // 10 detik

#define SOIL_MOISTURE_THRESHOLD (64)
#define SOIL_MOISTURE_SENSOR_PIN (33)

#define RELAY_PIN (26)

#define WATERFLOW_SENSOR_PIN (27)

#define DHT_SENSOR_PIN (4)
#define DALLAS_SENSOR_PIN (32)

bool wifiStatus = 0, onRunning = false;
int lastRun = 61;

TaskHandle_t Task1;
TaskHandle_t Task2;

WaterFlow debit;

SoilMosture kelembaban(SOIL_MOISTURE_SENSOR_PIN);
ReadDHT dht;

bool isTaskRunning = false;

void Task1Func(void *Parameters)
{

  Penyiram penyiram(RELAY_PIN, SOIL_MOISTURE_THRESHOLD);

  for (;;)
  {
    if (onRunning)
    {
      if (!isTaskRunning)
      {
        penyiram.start();
        isTaskRunning = true;
      }

      penyiram.run(kelembaban.getKelembaban(), &isTaskRunning);
      debit.run();

      onRunning = isTaskRunning;
    }
    delay(5);
  }
}

void Task2Func(void *Parameters)
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

  FirebaseController fbData(SOIL_MOISTURE_SENSOR_PIN);

  unsigned int lastmillis1 = millis();
  unsigned int lastmillis2 = millis();

  bool lastOnRunning = onRunning;

  for (;;)
  {
    if (millis() - lastmillis2 >= DELAY_GET_SERVICE)
    {
      if (fbData.isServiceOn())
      {
        if (onRunning != lastOnRunning)
        {
          Serial.println("ganti status penyiraman");

          Serial.println(debit.getTotalMilliLiters());

          fbData.sendNotification(debit.getTotalMilliLiters());
          debit.stopReading();

          lastOnRunning = onRunning;
        }

        if (fbData.isScheduleRun(onRunning, lastRun))
        {
          Serial.println("penyiraman berhasil");
          onRunning = true;
          lastOnRunning = onRunning;

          lastRun = fbData.getMinute();

          debit.startReading();
        }

        if (fbData.isAutomasiRun(onRunning))
        {
          Serial.println("penyiraman berhasil");
          onRunning = true;
          lastOnRunning = onRunning;

          debit.startReading();
        }
      }
      lastmillis2 = millis();
    }

    if (millis() - lastmillis1 >= DELAY_UPDATE_FIREBASE)
    {      
      fbData.setSensorData(dht.KelembabanUdara(), dht.SuhuUdara());
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

void IRAM_ATTR pulseCounter()
{
  pulseCount++;
}

void setup()
{
  Serial.begin(115200);

  attachInterrupt(digitalPinToInterrupt(WATERFLOW_SENSOR_PIN), pulseCounter, FALLING);

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
  // dht.SuhuUdara();
  // delay(1000);
}