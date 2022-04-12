#include <Arduino.h>
#include <FirebaseESP32.h>
#include <addons/RTDBHelper.h>
#include <esp_task_wdt.h>

#include "FirebaseConnection.h"
#include "FirebaseController.h"
#include "WifiConnection.h"
#include "WaterFlow.h"
#include "Penyiram.h"
#include "Sensors.h"

#define DELAY_UPDATE_FIREBASE (5000)
#define DELAY_GET_SERVICE (10000)
#define DELAY_DS18B20_UPDATE (2000)

#define SOIL_MOISTURE_THRESHOLD (35)
#define SOIL_MOISTURE_MINIMUM (20)
#define SOIL_MOISTURE_SENSOR_PIN (33)

#define RELAY_PIN (26)
#define WATERFLOW_SENSOR_PIN (27)

#define DHT_SENSOR_PIN (4)
#define DHTTYPE (DHT11)
#define DS18B20_PIN (32)

TaskHandle_t Task1;
TaskHandle_t Task2;

SemaphoreHandle_t xSemaphore = NULL;

WaterFlow waterFlowSensor(WATERFLOW_SENSOR_PIN);

Sensors sensors(DS18B20_PIN, SOIL_MOISTURE_SENSOR_PIN, DHT_SENSOR_PIN, DHTTYPE);

int lastRun = 61;

unsigned long lastEpoch = 0;

uint8_t totalRunTime = 0;
uint8_t runTime = 0;
uint8_t soilMoistureThreshold = SOIL_MOISTURE_THRESHOLD;

enum OwnerRun
{
  Automation = 0,
  Schedule,
};

struct Irigation
{
  OwnerRun taskOwner;
  bool isRunning = false;
  int lastRunning = 0;
} irigation;

void connect()
{
  WifiConnection connect;

  if (connect.getConfig())
  {
    connect.wifiConnect();
  }

  FirebaseConnection fbCon;
  if (WiFi.isConnected())
  {
    fbCon.getConfig();
    fbCon.begin();
  }
}

bool checkAutomation(FirebaseController *fbData)
{
  if (fbData->soilAutomationCheck(irigation.isRunning, sensors.getSoilTemperature()))
  {
    soilMoistureThreshold = fbData->getSoilMoistureThreshold();

    irigation.isRunning = Automation;
    irigation.isRunning = true;
    return true;
  }
  return false;
}

bool checkSchedule(FirebaseController *fbData)
{
  if (fbData->isScheduleRun(irigation.isRunning, lastRun))
  {
    runTime = fbData->scheduleRunTime();
    totalRunTime = 0;

    irigation.taskOwner = Schedule;
    irigation.isRunning = true;

    lastRun = fbData->getMinute();
    return true;
  }
  return false;
}

void Task1Func(void *Parameters)
{
  WaterPump waterPump(RELAY_PIN);
  ReadDallas dallas(DS18B20_PIN);

  bool isTaskRunning = false;

  unsigned long lastMillis = millis();

  for (;;)
  {
    xSemaphoreTake(xSemaphore, portMAX_DELAY);

    if (irigation.isRunning)
    {
      if (!isTaskRunning)
      {
        waterPump.start();
        isTaskRunning = true;
        lastMillis = millis();
      }

      if (irigation.taskOwner == Automation)
      {
        waterPump.run(sensors.getAirHumidity(), &isTaskRunning, soilMoistureThreshold);
      }
      else if (irigation.taskOwner == Schedule)
      {
        if (totalRunTime >= runTime)
        {
          waterPump.stop();
          isTaskRunning = false;
        }
        else if (millis() - lastMillis >= 60000)
        {
          totalRunTime++;
          lastMillis = millis();
        }
      }
      waterFlowSensor.read();

      irigation.isRunning = isTaskRunning;
    }
    xSemaphoreGive(xSemaphore);
    delay(250);
  }
}

void Task2Func(void *Parameters)
{
  connect();

  FirebaseController fbData;

  unsigned int lastmillis1 = millis();
  unsigned int lastmillis2 = millis();

  bool lastRunning = irigation.isRunning;

  for (;;)
  {
    xSemaphoreTake(xSemaphore, portMAX_DELAY);
    if (irigation.isRunning != lastRunning)
    {
      waterFlowSensor.stopReading();
      if (waterFlowSensor.getTotalMilliLiters() > 0)
      {
        fbData.updateSensorData(&sensors);
        delay(10);
        fbData.sendWaterUsageLog(waterFlowSensor.getTotalMilliLiters(), lastEpoch);
        delay(10);
        fbData.sendNotification(waterFlowSensor.getTotalMilliLiters());
      }

      lastRunning = irigation.isRunning;
    }
    xSemaphoreGive(xSemaphore);

    if (millis() - lastmillis2 >= DELAY_GET_SERVICE)
    {
      if (fbData.isServiceRun())
      {
        xSemaphoreTake(xSemaphore, portMAX_DELAY);
        if (!irigation.isRunning)
        {

          if (checkSchedule(&fbData) || checkAutomation(&fbData))
          {
            lastEpoch = fbData.getEpoch();
            waterFlowSensor.startReading();
          }
        }
        xSemaphoreGive(xSemaphore);
      }
      lastmillis2 = millis();
    }

    if (millis() - lastmillis1 >= DELAY_UPDATE_FIREBASE)
    {
      xSemaphoreTake(xSemaphore, portMAX_DELAY);
      fbData.updateSensorData(&sensors);
      xSemaphoreGive(xSemaphore);

      lastmillis1 = millis();
    }

    xSemaphoreTake(xSemaphore, portMAX_DELAY);
    if (irigation.isRunning && irigation.taskOwner == Schedule)
    {
      if (lastRun + 1 == fbData.getMinute())
      {
        lastRun = 61;
        fbData.updateSchedule();
      }
    }
    xSemaphoreGive(xSemaphore);
    delay(1);
  }
}

void setup()
{
  esp_task_wdt_init(30, false);

  xSemaphore = xSemaphoreCreateBinary();

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