/**
 * @file       lcd_task.cpp
 * @license    This library is released under the MIT License.
 * @version    0.1.0
 * @date       2025-11-30
 * @author     Tuan Nguyen
 *
 * @brief      Source file for LCD Task
 *
 */

/* Includes ----------------------------------------------------------- */
#include "lcd_task.h"
#include "globals.h"

#include "button_task.h"
#include "lcd_16x2.h" // For LCD_I2C Class

/* Private defines ---------------------------------------------------- */

/* Private enumerate/structure ---------------------------------------- */

/* Private macros ----------------------------------------------------- */

/* Public variables --------------------------------------------------- */

QueueHandle_t xDisplayQueue;

/* Private variables -------------------------------------------------- */

#ifdef LCD_MODULE
static LCD_I2C lcd(LCD_16x2_ADDR, 16, 2);
#endif

/* Task definitions ------------------------------------------- */
#ifdef LCD_MODULE
void vLcdTask(void *pvParameters)
{
  float   temp          = NAN;
  float   humidity      = NAN;
  uint8_t lux           = -1;
  uint8_t soil_moisture = -1;
  uint8_t fanSpeed      = 0;
  bool    ledState      = false;
  bool    pumpState     = false;

  DisplayMessage_t receivedMessage;
  ButtonMessage_t  receviedButtonMessage;

  QueueSetHandle_t xDisplayQueueSet = xQueueCreateSet(15);

  xQueueReset(xDisplayQueue);
  xQueueReset(xButtonQueue);

  xQueueAddToSet(xDisplayQueue, xDisplayQueueSet);
  xQueueAddToSet(xButtonQueue, xDisplayQueueSet);

  for (;;)
  {
    QueueSetMemberHandle_t xActivatedQueue = xQueueSelectFromSet(xDisplayQueueSet, portMAX_DELAY);

    if (xActivatedQueue == xDisplayQueue)
    {
      xQueueReceive(xDisplayQueue, &receivedMessage, 0);

      if (receivedMessage.updateSource == DISPLAY_UPDATE_SENSORS)
      {
        temp          = receivedMessage.sensorData.temperature;
        humidity      = receivedMessage.sensorData.humidity;
        lux           = receivedMessage.sensorData.lux;
        soil_moisture = receivedMessage.sensorData.soil_moisture;
      }
      else if (receivedMessage.updateSource == DISPLAY_UPDATE_ACTUATORS)
      {
        switch (receivedMessage.actuatorData.targetDevice)
        {
          case DEVICE_LED:
            ledState = receivedMessage.actuatorData.value;
            break;
          case DEVICE_FAN:
            fanSpeed = receivedMessage.actuatorData.value;
            break;
          case DEVICE_PUMP:
            pumpState = receivedMessage.actuatorData.value;
            break;
        }
      }
    } // End if (xActivatedQueue == xDisplayQueue)

    else if (xActivatedQueue == xButtonQueue)
    {
      xQueueReceive(xButtonQueue, &receviedButtonMessage, 0);

      if (receviedButtonMessage.type == BUTTON_SINGLE_CLICK)
      {
        lcd.updateScreenState(true);
      }
      else if (receviedButtonMessage.type == BUTTON_DOUBLE_CLICK)
      {
        lcd.updateScreenState(false);
      }
    } // End else if (xActivatedQueue == xButtonQueue)

    if (wifiConnected)
    {
      switch (lcd.getScreenState())
      {
  #ifdef DHT20_MODULE
        case LCD_SCREEN_DHT20:
          lcd.clear();
          lcd.print("Hum: ");
          if (isnan(humidity))
          {
            lcd.print("--.-");
          }
          else
          {
            lcd.print(humidity);
          }
          lcd.print(" %");
          lcd.setCursor(0, 1);
          lcd.print("Temp: ");
          if (isnan(temp))
          {
            lcd.print("--.-");
          }
          else
          {
            lcd.print(temp);
          }
          lcd.print(" *C");
          break;
  #endif

  #ifdef LIGHT_SENSOR_MODULE
        case LCD_SCREEN_LIGHT:
          lcd.clear();
          lcd.print("Light level: ");
          if (lux == -1)
          {
            lcd.print("--.-");
          }
          else
          {
            lcd.print(lux);
            lcd.progressBar(1, lux);
          }
          break;
  #endif

  #ifdef SOIL_MOISTURE_MODULE
        case LCD_SCREEN_SOIL_MOISTURE:
          lcd.clear();
          lcd.print("Soil Moisture: ");
          lcd.setCursor(0, 1);
          if (soil_moisture == -1)
          {
            lcd.print("--.-");
          }
          else
          {
            lcd.print(soil_moisture);
          }
          lcd.print("%");
          break;
  #endif

  #ifdef PUMPING_MOTOR_MODULE
        case LCD_SCREEN_PUMPING_MOTOR:
          lcd.clear();
          lcd.print("Pumping Motor: ");
          lcd.print(pumpState);
          break;
  #endif

  #ifdef MINI_FAN_MODULE
        case LCD_SCREEN_MINIFAN:
          lcd.clear();
          lcd.print("Fan Speed: ");
          lcd.print(fanSpeed);
          lcd.print("%");
          break;
  #endif

  #ifdef LED_RGB_MODULE
        case LCD_SCREEN_LED_RGB:
          lcd.clear();
          lcd.print("LED RGB: ");
          lcd.print(ledState);
          break;
  #endif

        default:
          lcd.clear();
          lcd.print("Blank screen");
          break;
      } // End switch (lcd.getScreenState())
    } // End if wifiConnected
  } // End if for(;;)
}

void lcdTaskSetup()
{
  lcd.begin(&Wire);
  lcd.display();
  lcd.backlight();
  lcd.clear();

  // Splash Screen
  lcd.setCursor(0, 0);
  lcd.print("Greenhouse OS");
  lcd.setCursor(0, 1);
  lcd.print("Starting up...");

  xDisplayQueue = xQueueCreate(5, sizeof(DisplayMessage_t));
  if (xDisplayQueue != NULL)
  {
    xTaskCreate(vLcdTask, "LCD Task", 4096, NULL, 2, NULL);
  }
}
#endif // LCD_MODULE
       /* End of file -------------------------------------------------------- */