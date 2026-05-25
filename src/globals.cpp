/**
 * @file       globals.cpp
 * @license    This library is released under the MIT License.
 * @version    0.1.0
 * @date       2026-05-25
 * @author     Tuan Nguyen
 *
 * @brief      Source file for GLOBALS library
 *
 */

/* Includes ----------------------------------------------------------- */

#include "globals.h"

/* Private defines ---------------------------------------------------- */

/* Private enumerate/structure ---------------------------------------- */

/* Private macros ----------------------------------------------------- */

/* Public variables --------------------------------------------------- */

bool wifiConnected = false;

AutoControlConfig_t autoControlConfig;

SemaphoreHandle_t xAutoConfigMutex = xSemaphoreCreateMutex();

/* Private variables -------------------------------------------------- */

/* Class method definitions-------------------------------------------- */

/* Private function prototypes ---------------------------------------- */

/* End of file -------------------------------------------------------- */