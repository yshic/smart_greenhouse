/**
 * @file       soil_moisture.h
 * @license    This library is released under the MIT License
 * @version    0.1.0
 * @date       2024-11-01
 * @author     Tuan Nguyen
 *
 * @brief      Header for soil moisture sensor library
 *
 */

/* Define to prevent recursive inclusion ------------------------------ */
#ifndef SOIL_MOISTURE_H
  #define SOIL_MOISTURE_H

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

/* Class Declaration -------------------------------------------------- */

/**
 * @brief Manages soil moisture measurements using an analog soil moisture sensor.
 *
 * The `SoilMoisture` class provides functionality for reading and processing soil moisture data from an
 * analog sensor connected to an Arduino pin. It supports raw analog readings, mapped values, threshold
 * detection, averaging, and callback triggering for threshold events.
 *
 * ### Features:
 *
 * - Reads raw analog soil moisture values with 10-bit resolution.
 *
 * - Maps raw values to a user-defined range (e.g., 0–100).
 *
 * - Detects if soil moisture exceeds a specified threshold.
 *
 * - Computes average readings over multiple samples.
 *
 * - Supports callback functions for threshold-crossing events.
 *
 * ### Usage:
 *
 * Instantiate the class with the appropriate analog pin. Call `read()` to retrieve raw sensor data or
 * `readAndMap()` for scaled values. Use `isAboveThreshold()` for conditional checks, `getAverageReading()`
 * for smoothed data, or `onThresholdCross()` to set up event-driven callbacks. Regularly call `read()` to
 * update sensor values.
 *
 * ### Dependencies:
 *
 * - Requires an Arduino-compatible board with analog input support (10-bit ADC).
 *
 * - Sensor must be connected to a valid analog pin.
 *
 * - Depends on `bsp_gpio.h` for GPIO operations (`bspGpioAnalogRead`).
 *
 * - Callback functions must be defined before use with `onThresholdCross()`.
 */
class SoilMoisture
{
public:
  /**
   * @brief Constructor to initialize the SoilMoisture sensor with a specified pin.
   *
   * Configures the SoilMoisture sensor to use the provided analog pin for reading moisture levels.
   *
   * @param[in] pin The analog pin connected to the soil moisture sensor.
   *
   * @attention Ensure the pin is a valid analog input pin on the microcontroller.
   */
  SoilMoisture(int pin);

  /**
   * @brief Reads the analog value from the soil moisture sensor.
   *
   * Uses the `bspGpioAnalogRead` function to retrieve the raw 10-bit analog value (0–1023) from the sensor
   * and computes a percentage value (0–100), storing both internally.
   *
   * @param[in] None
   *
   * @attention Ensure the sensor is properly connected to the specified pin.
   */
  void read();

  /**
   * @brief Reads and maps the sensor value to a specified range.
   *
   * Reads the raw analog value from the sensor and maps it to a user-defined range (e.g., 0–100) using linear
   * scaling.
   *
   * @param[in] minValue Minimum value of the output range.
   * @param[in] maxValue Maximum value of the output range.
   *
   * @attention Ensure `minValue` is less than `maxValue` and the sensor is connected.
   *
   * @return uint32_t The mapped sensor value within the specified range.
   */
  uint32_t readAndMap(int minValue, int maxValue);

  /**
   * @brief Checks if the soil moisture level exceeds a threshold.
   *
   * Compares the current raw sensor value against a specified threshold to determine if the moisture level is
   * above it.
   *
   * @param[in] threshold The threshold value for soil moisture (0–1023).
   *
   * @attention Requires a prior call to `read()` to update the sensor value.
   *
   * @return bool `true` if the moisture level is above the threshold, `false` otherwise.
   */
  bool isAboveThreshold(int threshold);

  /**
   * @brief Retrieves the raw analog soil moisture value.
   *
   * Returns the most recent raw 10-bit analog value (0–1023) read from the sensor.
   *
   * @param[in] None
   *
   * @attention Requires a prior successful call to `read()`.
   *
   * @return int The raw analog sensor value.
   */
  int getMoisture();

  /**
   * @brief Retrieves the soil moisture as a percentage.
   *
   * Returns the internally stored percentage value (0–100) computed from the raw analog sensor value.
   *
   * @param[in] None
   *
   * @attention Requires a prior successful call to `read()`.
   *
   * @return int The soil moisture as a percentage.
   */
  int getMoisturePercentage();

  /**
   * @brief Computes the average sensor reading over multiple samples.
   *
   * Takes multiple readings from the sensor with a 10ms delay between each and returns their average to
   * reduce noise.
   *
   * @param[in] samples Number of samples to average (default: 10).
   *
   * @attention Ensure the sensor is connected and `samples` is a positive integer.
   *
   * @return int The averaged raw sensor value.
   */
  int getAverageReading(int samples = 10);

  /**
   * @brief Sets a callback function for threshold-crossing events.
   *
   * Configures a callback function to be triggered when the soil moisture level crosses the specified
   * threshold, invoking the callback if the threshold is currently exceeded.
   *
   * @param[in] threshold The threshold value for triggering the callback (0–1023).
   * @param[in] callback The function to call when the threshold is crossed.
   *
   * @attention The callback function must be defined, have no parameters, and no return value. Requires a
   * prior call to `read()` for accurate threshold detection.
   */
  void onThresholdCross(int threshold, void (*callback)());

private:
  int      _pin;                 /** SIG pin of the Soil Moisture Sensor */
  uint32_t sensorValue[2];       /** Value from the sensor */
                                 // Index 0 : Raw analog value
                                 // Index 1 : Percentage value
  void (*_callback)() = nullptr; /** Callback function */
};

#endif // SOIL_MOISTURE_H

/* End of file -------------------------------------------------------- */