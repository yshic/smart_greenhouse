/**
 * @file       wifi_task.cpp
 * @license    This library is released under the MIT License.
 * @version    0.1.0
 * @date       2025-02-01
 * @author     Tuan Nguyen
 *
 * @brief      Source file for WiFi Task
 *
 */

/* Includes ----------------------------------------------------------- */
#include "wifi_task.h"
#include "globals.h"
#include <WiFi.h>

#include "secrets.h"

/* Private defines ---------------------------------------------------- */

/* Private enumerate/structure ---------------------------------------- */

/* Private macros ----------------------------------------------------- */

/* Public variables --------------------------------------------------- */

/* Private variables -------------------------------------------------- */

/* Task definitions ------------------------------------------- */
void onWifiEvent(WiFiEvent_t event)
{
  switch (event)
  {
    case ARDUINO_EVENT_WIFI_STA_START:
#ifdef DEBUG_PRINT
      Serial.println("Wi-Fi Station started. Connecting...");
#endif
      break;

    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
#ifdef DEBUG_PRINT
      Serial.print("Connected to Wi-Fi. IP: ");
      Serial.println(WiFi.localIP());
#endif
      wifiConnected = true;
      break;

    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
#ifdef DEBUG_PRINT
      Serial.println("Wi-Fi disconnected. Attempting to reconnect...");
#endif
      wifiConnected = false;

      // Tell the hardware driver to keep trying in the background
      WiFi.reconnect();
      break;

    default:
      break;
  }
}

void wifiSetup()
{
  WiFi.onEvent(onWifiEvent);
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(true);

  WiFi.begin(DEFAULT_SSID_HOME, DEFAULT_PASSWORD_HOME);
}

/* End of file -------------------------------------------------------- */