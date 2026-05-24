/**
 * @file       actuators_task.cpp
 * @license    This library is released under the MIT License.
 * @version    0.1.0
 * @date       2025-04-28
 * @author     Tuan Nguyen
 *
 * @brief      Source file for actuators_task.cpp library
 *
 */

/* Includes ----------------------------------------------------------- */

/* Private defines ---------------------------------------------------- */
#include "actuators_task.h"
#include "globals.h"

#include "lcd_task.h"          // For DisplayMessage_t, DisplayUpdateSource_t, xDisplayQueue
#include "mini_fan.h"          // For MiniFan Class
#include "usb_switch.h"        // For UsbSwitch Class
#include <Adafruit_NeoPixel.h> // For Adafruit_NeoPixel Class

/* Private enumerate/structure ---------------------------------------- */

/* Private macros ----------------------------------------------------- */

/* Public variables --------------------------------------------------- */

QueueHandle_t xActuatorQueue;

QueueHandle_t xDeviceStatusQueue;

/* Private variables -------------------------------------------------- */

#ifdef LED_RGB_MODULE
static Adafruit_NeoPixel rgb(4, LED_RGB_PIN, NEO_GRB + NEO_KHZ800);
#endif

#ifdef MINI_FAN_MODULE
static MiniFan miniFan(MINI_FAN_PIN);
#endif

#ifdef PUMPING_MOTOR_MODULE
static UsbSwitch pumpingMotor(USB_SWITCH_PIN_1, USB_SWITCH_PIN_2);
#endif

/* Task definitions-------------------------------------------- */

void vActuatorTask(void *pvParameters)
{
  ActuatorCommand_t receivedCommand;
  DisplayMessage_t  displayMessage;

  for (;;)
  {
    if (xQueueReceive(xActuatorQueue, &receivedCommand, pdMS_TO_TICKS(10)))
    {
      switch (receivedCommand.targetDevice)
      {
        case DEVICE_LED:
#ifdef LED_RGB_MODULE
          if (receivedCommand.value == 1) // ON
          {
            rgb.setPixelColor(0, rgb.Color(255, 102, 0));
            rgb.setPixelColor(1, rgb.Color(255, 102, 0));
            rgb.setPixelColor(2, rgb.Color(255, 102, 0));
            rgb.setPixelColor(3, rgb.Color(255, 102, 0));
          }
          else // OFF
          {
            rgb.setPixelColor(0, rgb.Color(0, 0, 0));
            rgb.setPixelColor(1, rgb.Color(0, 0, 0));
            rgb.setPixelColor(2, rgb.Color(0, 0, 0));
            rgb.setPixelColor(3, rgb.Color(0, 0, 0));
          }
          rgb.show();

          if (xDeviceStatusQueue != NULL)
          {
            xQueueSend(xDeviceStatusQueue, &receivedCommand, pdMS_TO_TICKS(10));
          }

          displayMessage.updateSource = DISPLAY_UPDATE_ACTUATORS;
          displayMessage.actuatorData = receivedCommand;
          if (xDisplayQueue != NULL)
          {
            xQueueSend(xDisplayQueue, &displayMessage, pdMS_TO_TICKS(10));
          }
#endif
          break;

        case DEVICE_FAN: {
#ifdef MINI_FAN_MODULE
          int safeSpeed = constrain(receivedCommand.value, 0, 100);
          miniFan.setFanSpeedPercentage(safeSpeed);

          if (xDeviceStatusQueue != NULL)
          {
            xQueueSend(xDeviceStatusQueue, &receivedCommand, pdMS_TO_TICKS(10));
          }

          displayMessage.updateSource = DISPLAY_UPDATE_ACTUATORS;
          displayMessage.actuatorData = receivedCommand;
          if (xDisplayQueue != NULL)
          {
            xQueueSend(xDisplayQueue, &displayMessage, pdMS_TO_TICKS(10));
          }
#endif
          break;
        }

        case DEVICE_PUMP:
#ifdef PUMPING_MOTOR_MODULE
          if (receivedCommand.value == 1)
          {
            pumpingMotor.setOutputValue(1, 255);
          }
          else
          {
            pumpingMotor.setOutputValue(1, 0);
          }

          if (xDeviceStatusQueue != NULL)
          {
            xQueueSend(xDeviceStatusQueue, &receivedCommand, pdMS_TO_TICKS(10));
          }

          displayMessage.updateSource = DISPLAY_UPDATE_ACTUATORS;
          displayMessage.actuatorData = receivedCommand;
          if (xDisplayQueue != NULL)
          {
            xQueueSend(xDisplayQueue, &displayMessage, pdMS_TO_TICKS(10));
          }
#endif // PUMPING_MOTOR_MODULE
          break;

        default:
          break;
      }
    }
  }
}

void actuatorTaskSetup()
{
#ifdef LED_RGB_MODULE
  rgb.begin();
#endif

  xActuatorQueue     = xQueueCreate(10, sizeof(ActuatorCommand_t));
  xDeviceStatusQueue = xQueueCreate(10, sizeof(ActuatorCommand_t));

  if (xActuatorQueue != NULL && xDeviceStatusQueue != NULL)
  {
    xTaskCreate(vActuatorTask, "Actuator Task", 4096, NULL, 2, NULL);
  }
}

/* Private function prototypes ---------------------------------------- */

/* End of file -------------------------------------------------------- */