/**
 * @file       button_task.cpp
 * @license    This library is released under the MIT License.
 * @version    0.1.0
 * @date       2025-02-01
 * @author     Tuan Nguyen
 *
 * @brief      Source file for Button Task
 *
 */

/* Includes ----------------------------------------------------------- */
#include "button_task.h"
#include "globals.h"

#include "button.h" // For ButtonHandler Class

#ifdef BUTTON_MODULE
/* Private defines ---------------------------------------------------- */

/* Private enumerate/structure ---------------------------------------- */

/* Private macros ----------------------------------------------------- */

/* Public variables --------------------------------------------------- */

QueueHandle_t xButtonQueue;

/* Private variables -------------------------------------------------- */

static ButtonHandler button(BUTTON_PIN, false, true);

/* Task definitions ------------------------------------------- */
void vButtonTask(void *pvParameters)
{
  TickType_t xLastWakeTime = xTaskGetTickCount();
  for (;;)
  {
    // Serial.print(bspGpioDigitalRead(BUTTON_PIN));
    button.update();
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(DELAY_BUTTON));
  }
}

void singleClickCallback()
{
  ButtonMessage_t clickType;
  clickType.type = BUTTON_SINGLE_CLICK;
  if (xButtonQueue != NULL)
  {
    xQueueSend(xButtonQueue, &clickType, pdMS_TO_TICKS(10));
  }
}

void doubleClickCallback()
{
  ButtonMessage_t clickType;
  clickType.type = BUTTON_DOUBLE_CLICK;
  if (xButtonQueue != NULL)
  {
    xQueueSend(xButtonQueue, &clickType, pdMS_TO_TICKS(10));
  }
}

void buttonTaskSetup()
{
  xButtonQueue = xQueueCreate(10, sizeof(ButtonMessage_t));
  // Button timing config
  button.setDebounceDuration(BUTTON_DEBOUNCE_DURATION);
  button.setDoubleClickInterval(BUTTON_DOUBLE_CLICK_INTERVAL);
  button.setHoldDuration(BUTTON_HOLD_DURATION);

  // Attach callback
  button.attachSingleClickCallback(singleClickCallback);
  button.attachDoubleClickCallback(doubleClickCallback);

  gpio_wakeup_enable((gpio_num_t) BUTTON_PIN, GPIO_INTR_LOW_LEVEL);
  esp_sleep_enable_gpio_wakeup();

  if (xButtonQueue != NULL)
  {
    xTaskCreate(vButtonTask, "Button Task", 4096, NULL, 1, NULL);
  }
}
#endif
/* End of file -------------------------------------------------------- */