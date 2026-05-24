/**
 * @file       dht20.h
 * @license    This library is released under the MIT License.
 * @version    0.3.0
 * @date       2024-11-01
 * @author     Tuan Nguyen
 *
 * @brief      Header file for DHT20 library
 *
 */

/* Define to prevent recursive inclusion ------------------------------ */
#ifndef DHT20_H
  #define DHT20_H

  /* Includes ----------------------------------------------------------- */
  #if ARDUINO >= 100
    #include "Arduino.h"
  #else
    #include "WProgram.h"
  #endif

  /* Public defines ----------------------------------------------------- */
  #define DHT20_LIB_VERSION       (F("0.1.0"))

  #define DHT20_I2C_ADDR          0x38
  #define DHT20_HUMIDITY_INDEX    0
  #define DHT20_TEMPERATURE_INDEX 1

/* Public enumerate/structure ----------------------------------------- */
typedef enum
{
  DHT20_OK = 0,       /* No error */
  DHT20_ERR,          /* Generic error */
  DHT20_ERR_INIT,     /* Initialization error */
  DHT20_ERR_RESET,    /* Reset error */
  DHT20_TIMEOUT,      /* Timeout error*/
  DHT20_ERR_I2C_READ, /* I2C Read error */
  DHT20_ERR_I2C_WRITE /* I2C Write error */
} dht20_error_t;
/* Public macros ------------------------------------------------------ */

/* Public variables --------------------------------------------------- */

/* Class Declaration -------------------------------------------------- */

/**
 * @brief Manages communication with the DHT20 temperature and humidity sensor.
 *
 * The `DHT20` class provides an interface for interacting with the DHT20 sensor using I2C communication.
 * It allows users to initialize the sensor, read temperature and humidity data, and handle sensor status.
 *
 * ### Features:
 *
 * - Initialization and status management.
 *
 * - Reading humidity and temperature values.
 *
 * - Handling errors related to sensor communication.
 *
 * - Provides retry mechanisms for stable sensor interaction.
 *
 * ### Usage:
 *
 * The class should be instantiated, followed by calling `begin()` to initialize the sensor.
 * Temperature and humidity data can be retrieved using `readTempAndHumidity()`, `getTemperature()`,
 * and `getHumidity()`. The class also includes internal mechanisms to manage sensor resets and diagnostics.
 *
 * ### Dependencies:
 *
 * - Requires I2C communication for data retrieval.
 *
 * - Ensure correct wiring for stable operation.
 */
class DHT20
{
public:
  DHT20();

  /**
   * @brief  Initializes the DHT20 sensor.
   *
   * This function initializes the DHT20 sensor by waiting for a brief period after power-on and then
   * resetting the sensor.
   *
   * @param[in]     None
   *
   * @attention  Ensure that the sensor is correctly connected and powered on before calling this function.
   *             This function includes a 100ms delay to allow the sensor to stabilize after power-on.
   *
   * @return
   *  - `DHT20_OK`: Initialization success.
   */
  dht20_error_t begin();

  /**
   * @brief  Reads temperature and humidity data from the DHT20 sensor.
   *
   * This function attempts to read temperature and humidity data from the DHT20 sensor.
   * It retries the operation multiple times if the sensor is not ready or data is not available.
   * The raw sensor values are then converted to floating-point temperature and humidity values.
   *
   * @param[in]     None
   *
   * @attention  Ensure that the sensor is correctly connected and that I2C communication is functioning.
   *             This function may block for short periods while waiting for the sensor to be ready and for
   * data to be available.
   *
   * @return
   *  - `DHT20_OK`: Success
   *
   *  - `DHT20_TIMEOUT`: Timeout, the sensor is not ready or data is not available after multiple attempts.
   */
  dht20_error_t readTempAndHumidity();

  /**
   * @brief  Get humidity data from the DHT20 sensor.
   *
   * @param[in]     None
   *
   * @return   int      The humidity data
   *
   */
  float getHumidity();

  /**
   * @brief  Get temperature data from the DHT20 sensor.
   *
   * @param[in]     None
   *
   * @return   int      The temperature data
   *
   */
  float getTemperature();

private:
  float sensorValue[2];

  /**
   * @brief  Reads temperature and humidity data from the DHT20 sensor.
   *
   * This function reads a 6-byte buffer from the DHT20 sensor, where the first byte is the status register,
   * and the remaining 5 bytes contain temperature and humidity data. It processes these bytes to extract
   * the humidity and temperature values.
   *
   * @param[in]     None
   * @param[out]    data  Pointer to a buffer where the temperature and humidity data will be stored.
   *                      - `data[HUMIDITY_INDEX]` will hold the processed humidity data.
   *                      - `data[TEMPERATURE_INDEX]` will hold the processed temperature data.
   *
   * @attention  Ensure that the sensor is properly connected and that the I2C communication is functioning.
   *             The function waits for 80 milliseconds for the measurement to complete, which might block
   *             other operations during this time.
   *
   * @return
   *  - `DHT20_OK`: Success
   *
   *  - `DHT20_ERR_I2C_WRITE`: Error writing to the I2C bus
   *
   *  - `DHT20_ERR_I2C_READ`: Error reading from the I2C bus
   *
   *  - `DHT20_ERR`: Sensor is still busy or not ready
   */
  dht20_error_t readTargetData(uint32_t *data);

  /**
   * @brief  Reads the status register of the DHT20 sensor.
   *
   * This function reads the status register byte from the DHT20 sensor and checks specific bits to determine
   * the status of the sensor.
   *
   * @param[in]     None
   *
   * @attention  Ensure that the sensor is correctly connected and that I2C communication is functioning.
   *             This function may print an error message if the byte read operation fails.
   *
   * @return
   *  - 1: If `(status byte & 0x18) == 0x18`
   *
   *  - 0: If `(status byte & 0x18) != 0x18`
   */
  int readStatus();

  /**
   * @brief Resets the DHT20 sensor.
   *
   * This function resets the sensor by writing to specific registers. The logic for resetting the sensor is
   * adapted from another source with modifications.
   * The function checks the sensor status and attempts to reset the registers. Use with care.
   *
   * @param[in]     None
   *
   * @note See datasheet 7.4 Sensor Reading Process, point 1.
   * @attention Use with care.
   *
   * @return
   * - 3: OK, all registers reset successfully.
   *
   * - 0, 1, 2: Error, not all registers reset.
   *
   * - 255: No reset needed.
   */
  int reset();

  /**
   * @brief  Resets a specific register of the DHT20 sensor.
   *
   * This function writes to a specific register to reset it. The logic is based on demo code provided by
   * AOSONG.
   *
   * @param[in]     reg  The register address to reset.
   *
   * @attention Ensure that the I2C communication is functioning correctly.
   *
   * @return
   *  - true: Register reset successfully.
   *  - false: Failed to reset the register.
   */
  bool _resetRegister(uint8_t reg);
};

#endif // DHT20_H

/* End of file -------------------------------------------------------- */