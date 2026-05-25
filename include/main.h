/**
 * @file       main.h
 * @license    This library is released under the MIT License.
 * @version    0.1.0
 * @date       2026-05-24
 * @author     Tuan Nguyen
 *
 * @brief      Header file for MAIN library
 */

/* Define to prevent recursive inclusion ------------------------------ */
#ifndef MAIN_H
  #define MAIN_H

  /* Includes ----------------------------------------------------------- */
  #if ARDUINO >= 100
    #include "Arduino.h"
  #else
    #include "WProgram.h"
  #endif

  #include "esp_pm.h"  // ESP Power Management Library
  #include "globals.h" // Global macro
  #include "utility.h" // Utility functions

// RTOS Task Setup
  #include "../src/tasks/actuators_task.h"
  #include "../src/tasks/button_task.h"
  #include "../src/tasks/iot_server_task.h"
  #include "../src/tasks/lcd_task.h"
  #include "../src/tasks/sensors_task.h"
  #include "../src/tasks/wifi_task.h"

  /* Public defines ----------------------------------------------------- */
  #define MAIN_LIB_VERSION (F("0.1.0"))

/* Public enumerate/structure ----------------------------------------- */

/* Public macros ------------------------------------------------------ */

/* Public variables --------------------------------------------------- */

/* Class Declaration -------------------------------------------------- */

#endif // MAIN_H

/* End of file -------------------------------------------------------- */