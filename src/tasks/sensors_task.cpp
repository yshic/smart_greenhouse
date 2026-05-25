/**
 * @file       sensors_task.cpp
 * @license    This library is released under the MIT License.
 * @version    0.1.0
 * @date       2025-11-29
 * @author     Tuan Nguyen
 *
 * @brief      Source file for Sensors Task
 *
 */

/* Includes ----------------------------------------------------------- */
#include "sensors_task.h"

#include "dht20.h"         // For DHT20 Class
#include "light_sensor.h"  // For LightSensor Class
#include "soil_moisture.h" // For SoilMoisture Class

#include "globals.h"
#include "lcd_task.h" // For DisplayMessage_t

/* Private defines ---------------------------------------------------- */

/* Private enumerate/structure ---------------------------------------- */

/* Private macros ----------------------------------------------------- */

/* Public variables --------------------------------------------------- */

QueueHandle_t xSensorQueue;

/* Private variables -------------------------------------------------- */
#ifdef DHT20_MODULE
static DHT20 dht20;
#endif

#ifdef LIGHT_SENSOR_MODULE
static LightSensor lightSensor(LIGHT_SENSOR_PIN);
#endif

#ifdef SOIL_MOISTURE_MODULE
SoilMoisture soilMoisture(SOIL_MOISTURE_PIN);
#endif

/* Task definitions ------------------------------------------- */

void vSensorsTask(void *pvParameters)
{
  TickType_t       xLastWakeTime = xTaskGetTickCount();
  const TickType_t xFrequency    = pdMS_TO_TICKS(SENSOR_POLL_RATE_MS);

  SensorData_t     currentReadings;
  DisplayMessage_t displayMessage;

  bool isFanOn  = false;
  bool isLedOn  = false;
  bool isPumpOn = false;

  for (;;)
  {
    memset(&currentReadings, 0, sizeof(SensorData_t));
    memset(&displayMessage, 0, sizeof(DisplayMessage_t));

#ifdef DHT20_MODULE
    dht20.readTempAndHumidity();
    currentReadings.temperature = dht20.getTemperature();
    currentReadings.humidity    = dht20.getHumidity();
#endif

#ifdef LIGHT_SENSOR_MODULE
    lightSensor.read();
    currentReadings.lux = lightSensor.getLightValuePercentage();
#endif

#ifdef SOIL_MOISTURE_MODULE
    soilMoisture.read();
    currentReadings.soil_moisture = soilMoisture.getMoisturePercentage();
#endif

    if (xSensorQueue != NULL)
    {
      xQueueSend(xSensorQueue, &currentReadings, pdMS_TO_TICKS(10));
    }

    displayMessage.updateSource = DISPLAY_UPDATE_SENSORS;
    displayMessage.sensorData   = currentReadings;
    if (xDisplayQueue != NULL)
    {
      xQueueSend(xDisplayQueue, &displayMessage, pdMS_TO_TICKS(10));
    }

    if (xAutoConfigMutex != NULL && xSemaphoreTake(xAutoConfigMutex, pdMS_TO_TICKS(50)) == pdTRUE)
    {
      if (autoControlConfig.autoFanFlag == true)
      {
        bool needsFan = (currentReadings.temperature >= autoControlConfig.temp_threshold);
        if (needsFan && !isFanOn)
        {
          ActuatorCommand_t cmd = {DEVICE_FAN, 80};
          xQueueSend(xActuatorQueue, &cmd, pdMS_TO_TICKS(0));
          isFanOn = true;
        }
        else if (!needsFan && isFanOn)
        {
          ActuatorCommand_t cmd = {DEVICE_FAN, 0};
          xQueueSend(xActuatorQueue, &cmd, pdMS_TO_TICKS(0));
          isFanOn = false;
        }
      }

      if (autoControlConfig.autoLedFlag == true)
      {
        bool needsLed = (currentReadings.lux <= autoControlConfig.light_threshold);
        if (needsLed && !isLedOn)
        {
          ActuatorCommand_t cmd = {DEVICE_LED, 1};
          xQueueSend(xActuatorQueue, &cmd, pdMS_TO_TICKS(0));
          isLedOn = true;
        }
        else if (!needsLed && isLedOn)
        {
          ActuatorCommand_t cmd = {DEVICE_LED, 0};
          xQueueSend(xActuatorQueue, &cmd, pdMS_TO_TICKS(0));
          isLedOn = false;
        }
      }

      if (autoControlConfig.autoPumpFlag == true)
      {
        bool needsPump = (currentReadings.soil_moisture <= autoControlConfig.moisture_threshold);
        if (needsPump && !isPumpOn)
        {
          ActuatorCommand_t cmd = {DEVICE_PUMP, 1};
          xQueueSend(xActuatorQueue, &cmd, pdMS_TO_TICKS(0));
          isPumpOn = true;
        }
        else if (!needsPump && isPumpOn)
        {
          ActuatorCommand_t cmd = {DEVICE_PUMP, 0};
          xQueueSend(xActuatorQueue, &cmd, pdMS_TO_TICKS(0));
          isPumpOn = false;
        }
      }
      xSemaphoreGive(xAutoConfigMutex);
    } // End if Semaphore check and take

    vTaskDelayUntil(&xLastWakeTime, xFrequency);
  }
}

void sensorsTaskSetup()
{
#ifdef DHT20_MODULE
  dht20.begin();
#endif

  xSensorQueue = xQueueCreate(5, sizeof(SensorData_t));
  if (xSensorQueue != NULL)
  {
    xTaskCreate(vSensorsTask, "Sensors Task", 4096, NULL, 2, NULL);
  }
}

/* End of file -------------------------------------------------------- */