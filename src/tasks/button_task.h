/**
 * @file       button_task.h
 * @license    This library is released under the MIT License.
 * @version    0.1.0
 * @date       2024-03-05
 * @author     Tuan Nguyen
 *
 * @brief      Header file for Button Task
 *
 */

/* Define to prevent recursive inclusion ------------------------------ */
#ifndef BUTTON_TASK_H
  #define BUTTON_TASK_H

  /* Includes ----------------------------------------------------------- */
  #if ARDUINO >= 100
    #include "Arduino.h"
  #else
    #include "WProgram.h"
  #endif

/* Public defines ----------------------------------------------------- */

  #define DELAY_BUTTON                 10

  #define BUTTON_DEBOUNCE_DURATION     80
  #define BUTTON_DOUBLE_CLICK_INTERVAL 300
  #define BUTTON_HOLD_DURATION         1000

/* Public enumerate/structure ----------------------------------------- */

typedef enum
{
  BUTTON_SINGLE_CLICK = 0,
  BUTTON_DOUBLE_CLICK,
  BUTTON_HOLD_START,
  BUTTON_HOLD_RELEASE
} ButtonClickType_t;

typedef struct
{
  ButtonClickType_t type;
} ButtonMessage_t;

/* Public macros ------------------------------------------------------ */

/* Public variables --------------------------------------------------- */

extern QueueHandle_t xButtonQueue;

/* Funtions Declaration -------------------------------------------------- */

void buttonTaskSetup();

#endif // BUTTON_TASK_H

/* End of file -------------------------------------------------------- */