/**
 * @file       lcd_task.h
 * @license    This library is released under the MIT License.
 * @version    0.1.0
 * @date       2025-11-30
 * @author     Tuan Nguyen
 *
 * @brief      Header file for LCD Task
 *
 */

/* Define to prevent recursive inclusion ------------------------------ */
#ifndef LCD_TASK_H
  #define LCD_TASK_H

  /* Includes ----------------------------------------------------------- */
  #if ARDUINO >= 100
    #include "Arduino.h"
  #else
    #include "WProgram.h"
  #endif

  #include "actuators_task.h" // for ActuatorCommand_t
  #include "sensors_task.h"   // for SensorData_t

/* Public defines ----------------------------------------------------- */

  #define DELAY_LCD 1000

/* Public enumerate/structure ----------------------------------------- */

typedef enum
{
  DISPLAY_UPDATE_SENSORS,
  DISPLAY_UPDATE_ACTUATORS
} DisplayUpdateSource_t;

typedef struct
{
  DisplayUpdateSource_t updateSource;
  union
  {
    SensorData_t      sensorData;
    ActuatorCommand_t actuatorData;
  };
} DisplayMessage_t;

/* Public macros ------------------------------------------------------ */

/* Public variables --------------------------------------------------- */

extern QueueHandle_t xDisplayQueue;

/* Funtions Declaration -------------------------------------------------- */

void lcdTaskSetup();

#endif // LCD_TASK_H

/* End of file -------------------------------------------------------- */