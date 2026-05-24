/**
 * @file       soil_moisture.cpp
 * @license    This library is released under the MIT License
 * @version    0.1.0
 * @date       2024-11-01
 * @author     Tuan Nguyen
 *
 * @brief      Source file for soil moisture sensor library
 *
 */

/* Includes ----------------------------------------------------------- */
#include "soil_moisture.h"
#include "bsp_gpio.h"

/* Private defines ---------------------------------------------------- */

/* Private enumerate/structure ---------------------------------------- */

/* Private macros ----------------------------------------------------- */

/* Public variables --------------------------------------------------- */

/* Private variables -------------------------------------------------- */

/* Class method Definitions ---------------------------------- */

// Constructor
SoilMoisture::SoilMoisture(int pin) : _pin(pin)
{
  pinMode(_pin, INPUT);
  sensorValue[0] = 0;
  sensorValue[1] = 0;
}

void SoilMoisture::read()
{
  sensorValue[0] = bspGpioAnalogRead(_pin);
  sensorValue[1] = map(sensorValue[0], 0, 1023, 0, 100);
  sensorValue[1] = constrain(sensorValue[1], 0, 100);
}

uint32_t SoilMoisture::readAndMap(int minValue, int maxValue)
{
  // Read the raw sensor value
  read();

  // Map the sensor value from 0-1023 to a range of minValue - maxValue
  uint32_t mappedValue = map(sensorValue[0], 0, 1023, minValue, maxValue);
  mappedValue          = constrain(mappedValue, minValue, maxValue);
  return mappedValue;
}

int SoilMoisture::getMoisture() { return sensorValue[0]; }

int SoilMoisture::getMoisturePercentage() { return sensorValue[1]; }

bool SoilMoisture::isAboveThreshold(int threshold) { return sensorValue[0] > threshold; }

int SoilMoisture::getAverageReading(int samples)
{
  long total = 0;
  for (int i = 0; i < samples; i++)
  {
    read();
    total += sensorValue[0];
    delay(10); // Small delay between readings
  }
  return total / samples;
}

void SoilMoisture::onThresholdCross(int threshold, void (*callback)())
{
  _callback = callback;
  if (isAboveThreshold(threshold) && _callback != nullptr)
  {
    _callback();
  }
}

/* Private function definitions --------------------------------------- */

/* End of file -------------------------------------------------------- */
