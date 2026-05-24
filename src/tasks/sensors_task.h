/**
 * @file       sensors_task.h
 * @license    This library is released under the MIT License.
 * @version    0.1.0
 * @date       2025-11-29
 * @author     Tuan Nguyen
 *
 * @brief      Header file for Sensors Task
 *
 */

/* Define to prevent recursive inclusion ------------------------------ */
#ifndef SENSORS_TASK_H
  #define SENSORS_TASK_H

  /* Includes ----------------------------------------------------------- */
  #if ARDUINO >= 100
    #include "Arduino.h"
  #else
    #include "WProgram.h"
  #endif

/* Public defines ----------------------------------------------------- */

  #define SENSOR_POLL_RATE_MS 10000

/* Public enumerate/structure ----------------------------------------- */

typedef struct
{
  float temperature;
  float humidity;
  int   lux;
  int   soil_moisture;
} SensorData_t;

/* Public macros ------------------------------------------------------ */

/* Public variables --------------------------------------------------- */

extern QueueHandle_t xSensorQueue;

/* Funtions Declaration -------------------------------------------------- */

void sensorsTaskSetup();

#endif // SENSORS_TASK_H

/* End of file -------------------------------------------------------- */