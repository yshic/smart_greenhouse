/**
 * @file       iot_server_task.cpp
 * @license    This library is released under the MIT License.
 * @version    0.1.0
 * @date       2025-02-01
 * @author     Tuan Nguyen
 *
 * @brief      Source file for IOT Server Task
 *
 */

/* Includes ----------------------------------------------------------- */
#include "iot_server_task.h"

#include "globals.h"
#include "secrets.h"
#include "utility.h"

#include "actuators_task.h"
#include "sensors_task.h"

#include <Arduino_MQTT_Client.h>
#include <WiFi.h>

#include <Attribute_Request.h>
#include <Server_Side_RPC.h>
#include <Shared_Attribute_Update.h>
#include <ThingsBoard.h>

#include <Espressif_Updater.h>
#include <OTA_Firmware_Update.h>

/* Private defines ---------------------------------------------------- */

/* Private enumerate/structure ---------------------------------------- */

/* Private macros ----------------------------------------------------- */

/* Public variables --------------------------------------------------- */

SemaphoreHandle_t xThingsBoardMutex;

/* Private variables -------------------------------------------------- */

// Firmware Title & Version
constexpr char CURRENT_FIRMWARE_TITLE[]   = "SMART_GREENHOUSE";
constexpr char CURRENT_FIRMWARE_VERSION[] = "1.0.1";

// Maximum amount of retries we attempt to download each firmware chunck over MQTT
constexpr uint8_t FIRMWARE_FAILURE_RETRIES = 12U;

// Size of each firmware chunck downloaded over MQTT,
// increased packet size, might increase download speed
constexpr uint16_t FIRMWARE_PACKET_SIZE = 4096U;

constexpr char     TOKEN[]          = COREIOT_TOKEN;
constexpr char     COREIOT_SERVER[] = "app.coreiot.io";
constexpr uint16_t COREIOT_PORT     = 1883U;

// Maximum size packets will ever be sent or received by the underlying MQTT client,
// if the size is to small messages might not be sent or received messages will be discarded
constexpr uint16_t MAX_MESSAGE_SEND_SIZE    = 512U;
constexpr uint16_t MAX_MESSAGE_RECEIVE_SIZE = 512U;
constexpr size_t   MAX_ATTRIBUTES           = 15U;

constexpr uint8_t SHARED_ATTRIBUTES_ARRAY_SIZE = 11U;
constexpr uint8_t CLIENT_ATTRIBUTES_ARRAY_SIZE = 3U;
constexpr uint8_t RPC_CALLBACK_ARRAY_SIZE      = 1U;

constexpr const char RPC_SWITCH_METHOD[]                = "setSwitchState";
constexpr char       RPC_REQUEST_CALLBACK_METHOD_NAME[] = "getCurrentTime";
constexpr uint8_t    MAX_RPC_SUBSCRIPTIONS              = 15U;
constexpr uint8_t    MAX_RPC_REQUEST                    = 15U;
constexpr uint64_t   REQUEST_TIMEOUT_MICROSECONDS       = 15000U * 1000U;

constexpr int16_t telemetrySendInterval = 30000U;

// DHT20 / SHT40
constexpr char TEMPERATURE_KEY[] = "temperature";
constexpr char HUMIDITY_KEY[]    = "humidity";

// Light Sensor
constexpr char ILLUMINANCE_KEY[] = "illuminance";

// Soil Humidity
constexpr char SOIL_MOISTURE_KEY[] = "soil_moisture";

// Attribute names

/***  Explanation
 * Client Attributes will be sent from device to cloud.
 * These attributes are what the dashboard widgets use to display the correct states and sync with the device.
 * Shared Attribute are data from cloud to the device. A callback function will be invoked when there's an
update to any Shared Attribute
 * The device will use these new data to control the appropriate peripherals.
 * At start, the device will sync the local state with the Shared Attribute on the cloud.
***/

constexpr char CLIENT_LED_STATE_ATTR[]  = "clientLedState";
constexpr char CLIENT_FAN_SPEED_ATTR[]  = "clientFanSpeed";
constexpr char CLIENT_PUMP_STATE_ATTR[] = "clientPumpState";

constexpr char CLIENT_AUTO_FAN_FLAG[]  = "clientAutoFanFlag";
constexpr char CLIENT_AUTO_LED_FLAG[]  = "clientAutoLedFlag";
constexpr char CLIENT_AUTO_PUMP_FLAG[] = "clientAutoPumpFlag";

constexpr char CLIENT_AUTO_FAN_TEMP_THRESHOLD[]      = "clientAutoFanThreshold";
constexpr char CLIENT_AUTO_LED_LIGHT_THRESHOLD[]     = "clientAutoLedThreshold";
constexpr char CLIENT_AUTO_PUMP_MOISTURE_THRESHOLD[] = "clientAutoPumpThreshold";

constexpr char SHARED_LED_STATE_ATTR[]  = "sharedLedState";
constexpr char SHARED_FAN_SPEED_ATTR[]  = "sharedFanSpeed";
constexpr char SHARED_PUMP_STATE_ATTR[] = "sharedPumpState";

constexpr char SHARED_AUTO_FAN_FLAG[]  = "sharedAutoFanFlag";
constexpr char SHARED_AUTO_LED_FLAG[]  = "sharedAutoLedFlag";
constexpr char SHARED_AUTO_PUMP_FLAG[] = "sharedAutoPumpFlag";

constexpr char SHARED_AUTO_FAN_TEMP_THRESHOLD[]      = "sharedAutoFanThreshold";
constexpr char SHARED_AUTO_LED_LIGHT_THRESHOLD[]     = "sharedAutoLedThreshold";
constexpr char SHARED_AUTO_PUMP_MOISTURE_THRESHOLD[] = "sharedAutoPumpThreshold";

constexpr char FW_TITLE_ATTR[]   = "fw_title";
constexpr char FW_VERSION_ATTR[] = "fw_version";

// Statuses for updating
bool currentFWSent = false;

// Initialize used APIs
OTA_Firmware_Update<>                                                 ota;
Server_Side_RPC<MAX_RPC_SUBSCRIPTIONS, MAX_RPC_REQUEST>               rpc;
Attribute_Request<2U, MAX_ATTRIBUTES>                                 attr_request;
Shared_Attribute_Update<SHARED_ATTRIBUTES_ARRAY_SIZE, MAX_ATTRIBUTES> shared_update;

const std::array<IAPI_Implementation *, 4U> apis = {&ota, &rpc, &attr_request, &shared_update};

// List of shared attributes for subscribing to their updates
constexpr std::array<const char *, SHARED_ATTRIBUTES_ARRAY_SIZE> SHARED_ATTRIBUTES_LIST = {
SHARED_LED_STATE_ATTR,
SHARED_PUMP_STATE_ATTR,
SHARED_FAN_SPEED_ATTR,
SHARED_AUTO_FAN_FLAG,
SHARED_AUTO_LED_FLAG,
SHARED_AUTO_PUMP_FLAG,
SHARED_AUTO_FAN_TEMP_THRESHOLD,
SHARED_AUTO_LED_LIGHT_THRESHOLD,
SHARED_AUTO_PUMP_MOISTURE_THRESHOLD,
FW_TITLE_ATTR,
FW_VERSION_ATTR};

WiFiClient          wifiClient;
Arduino_MQTT_Client mqttClient(wifiClient);
ThingsBoard tb(mqttClient, MAX_MESSAGE_RECEIVE_SIZE, MAX_MESSAGE_SEND_SIZE, Default_Max_Stack_Size, apis);
Espressif_Updater<> updater;

bool subscribed = false;

/* Private function definitions ------------------------------------------- */
#ifdef OTA_UPDATE_MODULE
void update_starting_callback()
{
  // Nothing to do
}

/// @brief End callback method that will be called as soon as the OTA firmware update, either finished
/// successfully or failed. Is meant to allow to either restart the device if the udpate was successfull or
/// to restart any stopped services before the update started in the subscribed update_starting_callback
/// @param success Either true (update successful) or false (update failed)
void finished_callback(const bool &success)
{
  if (success)
  {
  #ifdef DEBUG_PRINT
    Serial.println("Done, Reboot now");
  #endif // DEBUG_PRINT

  #ifdef ESP32
    esp_restart();
  #endif // ESP32

    return;
  }
  #ifdef DEBUG_PRINT
  Serial.println("Downloading firmware failed");
  #endif // DEBUG_PRINT
}

/// @brief Progress callback method that will be called every time our current progress of downloading the
/// complete firmware data changed, meaning it will be called if the amount of already downloaded chunks
/// increased. Is meant to allow to display a progress bar or print the current progress of the update into
/// the console with the currently already downloaded amount of chunks and the total amount of chunks
/// @param current Already received and processs amount of chunks
/// @param total Total amount of chunks we need to receive and process until the update has completed
void progress_callback(const size_t &current, const size_t &total)
{
  #ifdef DEBUG_PRINT
  Serial.printf("Progress %.2f%%\n", static_cast<float>(current * 100U) / total);
  #endif // DEBUG_PRINT
}
#endif // OTA_UPDATE_MODULE

/* Shared Atrribute Callback --------------------------------------------------- */
/// @brief Shared attribute update callback
/// @param data New value of shared attributes which is changed
void processSharedAttributes(const JsonObjectConst &data)
{
  for (auto it = data.begin(); it != data.end(); ++it)
  {
    const char *key = it->key().c_str();
    // OTA UPDATE
    if (strcmp(key, "fw_title") == 0 || strcmp(key, "fw_version") == 0)
    {
      String fwTitle   = data["fw_title"] | "";
      String fwVersion = data["fw_version"] | "";

      if (fwTitle == CURRENT_FIRMWARE_TITLE && compareVersion(CURRENT_FIRMWARE_VERSION, fwVersion) < 0)
      {
#ifdef DEBUG_PRINT
        Serial.println("New firmware available! Initiating OTA update...");
#endif // DEBUG_PRINT

        const OTA_Update_Callback callback(CURRENT_FIRMWARE_TITLE, CURRENT_FIRMWARE_VERSION, &updater,
                                           &finished_callback, &progress_callback, &update_starting_callback,
                                           FIRMWARE_FAILURE_RETRIES, FIRMWARE_PACKET_SIZE);
        ota.Start_Firmware_Update(callback);
      }
    }

    // LED RGB
    else if (strcmp(key, SHARED_LED_STATE_ATTR) == 0)
    {
      bool              ledState = it->value().as<bool>();
      ActuatorCommand_t sendCommand;
      sendCommand.targetDevice = DEVICE_LED;
      sendCommand.value        = ledState ? 1 : 0;

      // Send command to Actuator Task
      if (xActuatorQueue != NULL)
      {
        xQueueSend(xActuatorQueue, &sendCommand, pdMS_TO_TICKS(10));
      }
    }

    // PUMPING MOTOR
    else if (strcmp(key, SHARED_PUMP_STATE_ATTR) == 0)
    {
      bool pumpState = it->value().as<bool>();

      ActuatorCommand_t sendCommand;
      sendCommand.targetDevice = DEVICE_PUMP;
      sendCommand.value        = pumpState ? 1 : 0;

#ifdef DEBUG_PRINT
      Serial.print("Received set pumping state RPC. New state: ");
      Serial.println(pumpState);
#endif // DEBUG_PRINT

      // Send command to Actuator Task
      if (xActuatorQueue != NULL)
      {
        xQueueSend(xActuatorQueue, &sendCommand, pdMS_TO_TICKS(10));
      }
    }

    // FAN SPEED
    else if (strcmp(key, SHARED_FAN_SPEED_ATTR) == 0)
    {
      const uint8_t fanSpeed = it->value().as<uint8_t>();

      ActuatorCommand_t sendCommand;
      sendCommand.targetDevice = DEVICE_FAN;
      sendCommand.value        = (fanSpeed <= 100) ? fanSpeed : 100;

#ifdef DEBUG_PRINT
      Serial.printf("Fan speed is set to: %d\n", fanSpeed);
#endif // DEBUG_PRINT

      // Send command to Actuator Task
      if (xActuatorQueue != NULL)
      {
        xQueueSend(xActuatorQueue, &sendCommand, pdMS_TO_TICKS(10));
      }
    }

    // AUTO MODE

    // AUTO MODE FLAG
    else if (strcmp(key, SHARED_AUTO_FAN_FLAG) == 0)
    {
      if (xSemaphoreTake(xAutoConfigMutex, pdMS_TO_TICKS(50)) == pdTRUE)
      {
        autoControlConfig.autoFanFlag = it->value().as<bool>();
        xSemaphoreGive(xAutoConfigMutex);
      }
    }

    else if (strcmp(key, SHARED_AUTO_LED_FLAG) == 0)
    {
      if (xSemaphoreTake(xAutoConfigMutex, pdMS_TO_TICKS(50)) == pdTRUE)
      {
        autoControlConfig.autoLedFlag = it->value().as<bool>();
        xSemaphoreGive(xAutoConfigMutex);
      }
    }
    else if (strcmp(key, SHARED_AUTO_PUMP_FLAG) == 0)
    {
      if (xSemaphoreTake(xAutoConfigMutex, pdMS_TO_TICKS(50)) == pdTRUE)
      {
        autoControlConfig.autoPumpFlag = it->value().as<bool>();
        xSemaphoreGive(xAutoConfigMutex);
      }
    }

    // AUTO MODE THRESHOLD
    else if (strcmp(key, SHARED_AUTO_FAN_TEMP_THRESHOLD) == 0)
    {
      if (xSemaphoreTake(xAutoConfigMutex, pdMS_TO_TICKS(50)) == pdTRUE)
      {
        autoControlConfig.temp_threshold = it->value().as<float>();
        xSemaphoreGive(xAutoConfigMutex);
      }
    }
    else if (strcmp(key, SHARED_AUTO_LED_LIGHT_THRESHOLD) == 0)
    {
      if (xSemaphoreTake(xAutoConfigMutex, pdMS_TO_TICKS(50)) == pdTRUE)
      {
        autoControlConfig.light_threshold = it->value().as<uint8_t>();
        xSemaphoreGive(xAutoConfigMutex);
      }
    }
    else if (strcmp(key, SHARED_AUTO_PUMP_MOISTURE_THRESHOLD) == 0)
    {
      if (xSemaphoreTake(xAutoConfigMutex, pdMS_TO_TICKS(50)) == pdTRUE)
      {
        autoControlConfig.moisture_threshold = it->value().as<uint8_t>();
        xSemaphoreGive(xAutoConfigMutex);
      }
    }
  }
}
/* End of Shared Atrribute Callback ------------------------------------------------ */

// Attribute request did not receive a response in the expected amount of microseconds
void requestTimedOut()
{
#ifdef DEBUG_PRINT
  Serial.printf("Attribute request not receive response in (%llu) microseconds. Ensure client is connected "
                "to the MQTT broker and that the keys actually exist on the target device\n",
                REQUEST_TIMEOUT_MICROSECONDS);
#endif // DEBUG_PRINT
}

const Shared_Attribute_Callback<MAX_ATTRIBUTES>
attributes_callback(&processSharedAttributes, SHARED_ATTRIBUTES_LIST.cbegin(), SHARED_ATTRIBUTES_LIST.cend());

const Attribute_Request_Callback<MAX_ATTRIBUTES>
attribute_shared_request_callback(&processSharedAttributes, REQUEST_TIMEOUT_MICROSECONDS, &requestTimedOut,
                                  SHARED_ATTRIBUTES_LIST);
/* Task definitions ------------------------------------------- */

void iotServerTask(void *pvParameters)
{
  TickType_t xLastWakeTime = xTaskGetTickCount();
  for (;;)
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      bool needsSetup = false; // Tracks for fresh connection

      if (xSemaphoreTake(xThingsBoardMutex, portMAX_DELAY) == pdTRUE)
      {
        if (!tb.connected())
        {
          // If disconnected, attempt to connect
          if (tb.connect(COREIOT_SERVER, TOKEN, COREIOT_PORT))
          {
            needsSetup = true; // Connection successful, needs setup
          }
        }
        xSemaphoreGive(xThingsBoardMutex);
      }

      // If failed to connect, wait and retry
      if (!needsSetup && !tb.connected())
      {
        Serial.println("Failed to connect or disconnected");
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(5000));
        continue;
      }

      // Setup (Only runs ONCE immediately after a successful connect)
      if (needsSetup)
      {
#ifdef DEBUG_PRINT
        Serial.println("Connected to IoT server! Initiating setup...");
#endif
        if (xSemaphoreTake(xThingsBoardMutex, portMAX_DELAY) == pdTRUE)
        {
          // Send WiFi attributes
          tb.sendAttributeData("localIp", WiFi.localIP().toString().c_str());
          tb.sendAttributeData("ssid", WiFi.SSID().c_str());
          tb.sendAttributeData("bssid", WiFi.BSSIDstr().c_str());
          tb.sendAttributeData("macAddress", WiFi.macAddress().c_str());
          tb.sendAttributeData("channel", WiFi.channel());

          shared_update.Shared_Attributes_Unsubscribe();

          if (!shared_update.Shared_Attributes_Subscribe(attributes_callback))
          {
#ifdef DEBUG_PRINT
            Serial.println("Failed to subscribe for shared attribute updates");
#endif
            xSemaphoreGive(xThingsBoardMutex); // unlock before continuing
            continue;
          }

          // Request current value of shared attributes
          if (!attr_request.Shared_Attributes_Request(attribute_shared_request_callback))
          {
#ifdef DEBUG_PRINT
            Serial.println("Failed to request for shared attributes");
#endif
            xSemaphoreGive(xThingsBoardMutex); // unlock before continuing
            continue;
          }

          xSemaphoreGive(xThingsBoardMutex); // Successful setup, unlock
        }

// OTA UPDATE
#ifdef OTA_UPDATE_MODULE
        if (!currentFWSent)
        {
          currentFWSent = ota.Firmware_Send_Info(CURRENT_FIRMWARE_TITLE, CURRENT_FIRMWARE_VERSION);
        }
#endif // OTA_UPDATE_MODULE
      } // End if needsSetup

    } // End if (WiFi.status() == WL_CONNECTED)

    // Yield the CPU for 5 seconds before checking connection status again
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(5000));
  }
}

void sendTelemetryTask(void *pvParameters)
{
  SensorData_t      receivedReadings;
  ActuatorCommand_t deviceStatus;

  // Create a copy of autoControlConfig to send to ThingsBoard
  AutoControlConfig_t autoConfigCopy;

  // Queue large enough to hold both queue
  QueueSetHandle_t xNetworkQueueSet = xQueueCreateSet(15);

  // The queues have to be empty to be added to the Queue Set
  xQueueReset(xSensorQueue);
  xQueueReset(xDeviceStatusQueue);

  xQueueAddToSet(xSensorQueue, xNetworkQueueSet);       // Add the sensor queue to the queue set
  xQueueAddToSet(xDeviceStatusQueue, xNetworkQueueSet); // Add the device status queue to the queue set

  for (;;)
  {
    QueueSetMemberHandle_t xActivatedQueue = xQueueSelectFromSet(xNetworkQueueSet, portMAX_DELAY);
    bool                   hasSensorData   = false;
    bool                   hasDeviceData   = false;
    bool                   autoConfigCopiedSuccessfully = false;

    if (xActivatedQueue == xSensorQueue)
    {
      xQueueReceive(xSensorQueue, &receivedReadings, 0);
      hasSensorData = true;
    }
    else if (xActivatedQueue == xDeviceStatusQueue)
    {
      xQueueReceive(xDeviceStatusQueue, &deviceStatus, 0);
      hasDeviceData = true;
    }

    // Take the mutex and copy the config
    if (xAutoConfigMutex != NULL && xSemaphoreTake(xAutoConfigMutex, pdMS_TO_TICKS(50)) == pdTRUE)
    {
      autoConfigCopy = autoControlConfig;
      xSemaphoreGive(xAutoConfigMutex);
      autoConfigCopiedSuccessfully = true;
    }

    if (WiFi.status() == WL_CONNECTED)
    {
      if (xSemaphoreTake(xThingsBoardMutex, pdMS_TO_TICKS(100)) == pdTRUE)
      {
        if (tb.connected())
        {
          if (hasSensorData)
          {
#ifdef DHT20_MODULE
            if (!(isnan(receivedReadings.temperature) || isnan(receivedReadings.humidity)))
            {
  #ifdef DEBUG_PRINT
              Serial.print("Temperature: ");
              Serial.print(receivedReadings.temperature);
              Serial.print(" °C, Humidity: ");
              Serial.print(receivedReadings.humidity);
              Serial.println(" %");
  #endif // DEBUG_PRINT

              tb.sendTelemetryData(TEMPERATURE_KEY, receivedReadings.temperature);
              tb.sendTelemetryData(HUMIDITY_KEY, receivedReadings.humidity);
            }
#endif // DHT20_MODULE

#ifdef LIGHT_SENSOR_MODULE
            if (!(isnan(receivedReadings.lux)))
            {
  #ifdef DEBUG_PRINT
              Serial.print("Illuminance: ");
              Serial.print(receivedReadings.lux);
              Serial.println(" lux");
  #endif // DEBUG_PRINT
              tb.sendTelemetryData(ILLUMINANCE_KEY, receivedReadings.lux);
            }
#endif // LIGHT_SENSOR_MODULE

#ifdef SOIL_MOISTURE_MODULE
            if (!(isnan(receivedReadings.soil_moisture)))
            {
  #ifdef DEBUG_PRINT
              Serial.print("Soil Moisture: ");
              Serial.print(receivedReadings.soil_moisture);
              Serial.println(" %");
  #endif // DEBUG_PRINT
              tb.sendTelemetryData(SOIL_MOISTURE_KEY, receivedReadings.soil_moisture);
            }
#endif // SOIL_MOISTURE_MODULE
          } // End if hasSensorData

          if (hasDeviceData)
          {
            switch (deviceStatus.targetDevice)
            {
              case DEVICE_LED:
                tb.sendAttributeData(CLIENT_LED_STATE_ATTR, (bool) deviceStatus.value);
                break;
              case DEVICE_PUMP:
                tb.sendAttributeData(CLIENT_PUMP_STATE_ATTR, (bool) deviceStatus.value);
                break;
              case DEVICE_FAN:
                tb.sendAttributeData(CLIENT_FAN_SPEED_ATTR, deviceStatus.value);
                break;
            }
          } // End if hasDeviceData

          // Send attributes of autoControlConfig to Cloud
          if (autoConfigCopiedSuccessfully)
          {
            tb.sendAttributeData(CLIENT_AUTO_FAN_FLAG, autoConfigCopy.autoFanFlag);
            tb.sendAttributeData(CLIENT_AUTO_LED_FLAG, autoConfigCopy.autoLedFlag);
            tb.sendAttributeData(CLIENT_AUTO_PUMP_FLAG, autoConfigCopy.autoPumpFlag);
            tb.sendAttributeData(CLIENT_AUTO_FAN_TEMP_THRESHOLD, autoConfigCopy.temp_threshold);
            tb.sendAttributeData(CLIENT_AUTO_LED_LIGHT_THRESHOLD, autoConfigCopy.light_threshold);
            tb.sendAttributeData(CLIENT_AUTO_PUMP_MOISTURE_THRESHOLD, autoConfigCopy.moisture_threshold);
          }

          tb.sendAttributeData("rssi", WiFi.RSSI()); // Send WiFi signal strength

        } // End if tb.connected()
        xSemaphoreGive(xThingsBoardMutex);
      } // End if xSemaphoreTake
    } // End if WiFi/MQTT Connected
  } // End for(;;)
}

void thingsboardLoopTask(void *pvParameters)
{
  TickType_t lastWakeTime = xTaskGetTickCount();

  for (;;)
  {
    if (xSemaphoreTake(xThingsBoardMutex, pdMS_TO_TICKS(10)) == pdTRUE)
    {
      tb.loop();
      xSemaphoreGive(xThingsBoardMutex);
    }

    vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(50)); // Short delay for processing
  }
}

void iotServerSetup()
{
  xThingsBoardMutex = xSemaphoreCreateMutex();

  if (xThingsBoardMutex != NULL)
  {
    xTaskCreate(iotServerTask, "IOT Server Task", 8192, NULL, 4, NULL);
    xTaskCreate(sendTelemetryTask, "Send Telemetry Task", 8192, NULL, 4, NULL);
    xTaskCreate(thingsboardLoopTask, "ThingsBoard Loop Task", 8192, NULL, 3, NULL);
  }
}
/* End of file -------------------------------------------------------- */