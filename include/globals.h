/**
 * @file       globals.h
 * @version    0.1.0
 * @date       2025-02-01
 * @author     Tuan Nguyen
 *
 * @brief      Header file for global
 *
 */

/* Define to prevent recursive inclusion ------------------------------ */
#ifndef GLOBALS_H
  #define GLOBALS_H

/* Includes ----------------------------------------------------------- */

  #if ARDUINO >= 100
    #include "Arduino.h"
  #else
    #include "WProgram.h"
  #endif

/* Public defines ---------------------------------------------------- */

// DEBUGGING
// #define DEBUG_I2C

  #ifndef DEBUG_PRINT
    #define DEBUG_PRINT
  #endif // DEBUG_PRINT

// Choose Development Boards
//  #define YOLO_UNO
  #define XIAO_ESP32S3

// Communication
// Wireless
  #ifdef ESP32
    #define WIFI_MODULE
    #define IOT_SERVER_MODULE
    #define OTA_UPDATE_MODULE
  #endif

// Peripherals

// Sensors
  #define SENSORS_MODULE
  #define DHT20_MODULE
  #define LIGHT_SENSOR_MODULE
  #define SOIL_MOISTURE_MODULE

  // Display
  #define LCD_MODULE
  #define LED_RGB_MODULE

  // MOTORS / ACTUATORS
  #define ACTUATORS_MODULE
  #define MINI_FAN_MODULE
  #define BUTTON_MODULE
  #define PUMPING_MOTOR_MODULE

  // Define Pin
  #ifdef YOLO_UNO
    #define SDA_PIN          GPIO_NUM_11
    #define SCL_PIN          GPIO_NUM_12

    #define BUTTON_PIN       GPIO_NUM_6
    #define LED_RGB_PIN      GPIO_NUM_4
    #define MINI_FAN_PIN     GPIO_NUM_3
    #define SERVO_PIN        GPIO_NUM_2
    #define LIGHT_SENSOR_PIN GPIO_NUM_1
  #endif

  #ifdef XIAO_ESP32S3
    #define SDA_PIN
    #define SCL_PIN

    #define SOIL_MOISTURE_PIN A1
    #define BUTTON_PIN        D2
    #define LED_RGB_PIN       D0
    #define MINI_FAN_PIN      A8
    #define USB_SWITCH_PIN_1  D7
    #define USB_SWITCH_PIN_2  D6
    #define LIGHT_SENSOR_PIN  A9
  #endif

/* Public enumerate/structure ----------------------------------------- */

typedef struct
{
  float   temp_threshold;
  uint8_t light_threshold;
  uint8_t moisture_threshold;
  bool    autoFanFlag;
  bool    autoLedFlag;
  bool    autoPumpFlag;
} AutoControlConfig_t;

/* Public macros ------------------------------------------------------ */

/* Public variables --------------------------------------------------- */

extern bool wifiConnected;

extern AutoControlConfig_t autoControlConfig;

extern SemaphoreHandle_t xAutoConfigMutex;

#endif // GLOBALS_H

/* End of file -------------------------------------------------------- */