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

/* Private defines ---------------------------------------------------- */
// DEBUGGING
// #define DEBUG_I2C

  #ifndef DEBUG_PRINT
    #define DEBUG_PRINT
  #endif // DEBUG_PRINT

// Choose Development Boards
//  #define YOLO_UNO
  #define XIAO_ESP32S3

//  #define DEBUG_PRINT_RTOS_TIMING
// Communication
/* Wireless   --------------------------------------------------------- */
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

/* Includes ----------------------------------------------------------- */

  // RTOS
  #include "../src/tasks/wifi_task.h"

extern bool wifiConnected;

#endif // GLOBALS_H

/* End of file -------------------------------------------------------- */