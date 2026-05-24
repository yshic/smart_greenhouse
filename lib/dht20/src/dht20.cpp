/**
 * @file       dht20.cpp
 * @license    This library is released under the MIT License.
 * @version    0.3.0
 * @date       2024-11-01
 * @author     Tuan Nguyen
 *
 * @brief      Source file for DHT20 library
 *
 */

/* Includes ----------------------------------------------------------- */
#include "dht20.h"
#include "bsp_i2c.h"
#include "config.h" // Global config file

/* Private defines ---------------------------------------------------- */

/* Private enumerate/structure ---------------------------------------- */

/* Private macros ----------------------------------------------------- */

/* Public variables --------------------------------------------------- */

/* Private variables -------------------------------------------------- */

/* Class method definitions ------------------------------------------- */

// Constructor
DHT20::DHT20() {}

dht20_error_t DHT20::begin()
{
  DELAY(100); // Wait 100ms after power-on
  reset();
  return DHT20_OK;
}

/**** The logic for reseting the sensor is taken from another source with some modifications ****/
// Author: Rob Tillaart
// Date: 2024
// URL: https://github.com/RobTillaart/DHT20

int DHT20::reset(void)
{
  uint8_t count = 255;

  if (readStatus() == 0)
  {
    count++;
    if (_resetRegister(0x1B))
      count++;
    if (_resetRegister(0x1C))
      count++;
    if (_resetRegister(0x1E))
      count++;
    DELAY(10);
  }
  return count;
}

bool DHT20::_resetRegister(uint8_t reg)
{
  uint8_t values[3]    = {0};
  uint8_t resetParam[] = {0x00, 0x00};
  if (bspI2CWriteBytes(DHT20_I2C_ADDR, reg, resetParam, sizeof(resetParam)) != BSP_I2C_OK)
  {
    return false;
  }

  DELAY(5);

  bspI2CReadBytes(DHT20_I2C_ADDR, values, sizeof(values));

  DELAY(10);

  uint8_t retVal[2] = {values[1], values[2]};
  bspI2CWriteBytes(DHT20_I2C_ADDR, (0xB0 | reg), retVal, sizeof(retVal));

  DELAY(5);

  return true;
}
/**** END ****/

int DHT20::readStatus(void)
{
  bsp_i2c_error_t returnVal;
  uint8_t         status = 0;

  returnVal = bspI2CReadByte(DHT20_I2C_ADDR, 0x71, status);
  if (returnVal == BSP_I2C_ERR_READ)
  {
#ifdef DEBUG_PRINT
    Serial.println("Failed to read byte\n");
#endif // DEBUG_PRINT
  }
  if ((status & 0x18) == 0x18)
  {
    return 1;
  }
  else
  {
    return 0;
  }
}

dht20_error_t DHT20::readTargetData(uint32_t *data)
{
  uint8_t bytes[6]       = {0};          // Buffer to hold the 6 bytes
  uint8_t configParams[] = {0x33, 0x00}; // Command to start measurement

  // Send the measurement command
  if (bspI2CWriteBytes(DHT20_I2C_ADDR, 0xAC, configParams, sizeof(configParams)) != BSP_I2C_OK)
  {
    return DHT20_ERR_I2C_WRITE; // Write failed
  }

  // Wait for measurement to complete
  DELAY(80);

  // Read 6 bytes from the sensor

  if (bspI2CReadBytes(DHT20_I2C_ADDR, bytes, sizeof(bytes)) != BSP_I2C_OK)
  {
    return DHT20_ERR_I2C_READ; // Read failed
  }

  // Check the busy flag in the state byte (bytes[0])
  if (bytes[0] & 0x80)
  {
#ifdef DEBUG_PRINT
    Serial.println("Sensor is still busy");
#endif // DEBUG_PRINT

    return DHT20_ERR; // Sensor not ready
  }

  // Process humidity data (20 bits: bytes[1], bytes[2], bytes[3])
  data[DHT20_HUMIDITY_INDEX] = ((bytes[1] << 12) | (bytes[2] << 4) | (bytes[3] >> 4));

  // Process temperature data (20 bits: bytes[3], bytes[4], bytes[5])
  data[DHT20_TEMPERATURE_INDEX] = (((bytes[3] & 0x0F) << 16) | (bytes[4] << 8) | bytes[5]);

  return DHT20_OK; // Success
}

dht20_error_t DHT20::readTempAndHumidity()
{
  uint32_t target_val[2] = {0};
  uint32_t cnt           = 0;
  while (readStatus() == 0)
  {
    begin();
    DELAY(30);

    cnt++;
    if (cnt > 3)
    {
      return DHT20_TIMEOUT;
    }
  }
  // wait for data ready。
  while (readTargetData(target_val) != DHT20_OK)
  {
    cnt++;
    DELAY(50);

    if (cnt > 5)
    {
      return DHT20_TIMEOUT;
    }
  }
  sensorValue[DHT20_HUMIDITY_INDEX] =
  target_val[0] * 9.5367431640625e-5; // equivalent to target_val[0] * 100.0 / 1024 / 1024 but more
                                      // optimize by not using division
  sensorValue[DHT20_TEMPERATURE_INDEX] =
  target_val[1] * 1.9073486328125e-4 - 50; // target_val[1] * 200.0 / 1024 / 1024 - 50
  return DHT20_OK;
}

float DHT20::getHumidity() { return sensorValue[DHT20_HUMIDITY_INDEX]; }

float DHT20::getTemperature() { return sensorValue[DHT20_TEMPERATURE_INDEX]; }

/* Private function prototypes ---------------------------------------- */

/* End of file -------------------------------------------------------- */
