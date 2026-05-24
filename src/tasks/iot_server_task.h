/**
 * @file       iot_server_task.h
 * @license    This library is released under the MIT License.
 * @version    0.1.0
 * @date       2024-11-01
 * @author     Tuan Nguyen
 *
 * @brief      Header file for IOT Server Task
 *
 */

/* Define to prevent recursive inclusion ------------------------------ */
#ifndef IOT_SERVER_TASK_H
  #define IOT_SERVER_TASK_H

  /* Includes ----------------------------------------------------------- */
  #if ARDUINO >= 100
    #include "Arduino.h"
  #else
    #include "WProgram.h"
  #endif

/* Public defines ----------------------------------------------------- */

/* Public enumerate/structure ----------------------------------------- */

/* Public macros ------------------------------------------------------ */

/* Public variables --------------------------------------------------- */

/* Funtions Declaration -------------------------------------------------- */

void iotServerSetup();

#endif // IOT_SERVER_TASK_H

/* End of file -------------------------------------------------------- */