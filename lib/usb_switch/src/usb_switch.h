/**
 * @file       usb_switch.h
 * @license    This library is released under the MIT License
 * @version    0.1.0
 * @date       2024-11-01
 * @author     Tuan Nguyen
 *
 * @brief      Header for USB Switch Module library
 *
 */

/* Define to prevent recursive inclusion ------------------------------ */
#ifndef USB_SWITCH_H
  #define USB_SWITCH_H

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
 * @brief Manages a USB switch module with two PWM-controlled outputs.
 *
 * The `UsbSwitch` class provides functionality for controlling a USB switch module with two outputs
 * (USB_OUTPUT1 and USB_OUTPUT2) via PWM-capable pins. It supports setting output values as PWM or percentage,
 * toggling output states, and retrieving current values and statuses. The class maintains internal state for
 * both outputs.
 *
 * ### Features:
 *
 * - Controls two USB outputs with PWM values (0–255).
 *
 * - Sets and retrieves output values as percentages (0–100%).
 *
 * - Toggles outputs between ON (full PWM) and OFF states.
 *
 * - Tracks the ON/OFF status of each output.
 *
 * - Supports simultaneous control of both outputs.
 *
 * ### Usage:
 *
 * Instantiate the class with two PWM-capable pins for USB_OUTPUT1 and USB_OUTPUT2. Use `setOutputValue()` or
 * `setOutputValuePercentage()` to control output levels, `toggleOutput()` to switch states, and
 * `getStatus()`, `getOutputValue()`, or `getOutputValuePercentage()` to monitor outputs. Specify output
 * number (1, 2, or 0 for both) in method calls.
 *
 * ### Dependencies:
 *
 * - Requires an Arduino-compatible board with PWM support.
 *
 * - Outputs must be connected to valid PWM-capable pins (yellow wire for USB_OUTPUT1, white wire for
 * USB_OUTPUT2 if using Grove connector).
 *
 * - Depends on `bsp_gpio.h` for GPIO operations (`bspGpioPinMode`, `bspGpioAnalogWrite`,
 * `bspGpioAnalogRead`).
 */
class UsbSwitch
{
public:
  /**
   * @brief Constructor to initialize the USB Switch module with specified pins.
   *
   * Configures the provided pins as PWM outputs for controlling USB_OUTPUT1 and USB_OUTPUT2, initializing
   * both outputs to OFF.
   *
   * @param[in] pin1 The pin controlling USB_OUTPUT1 (yellow wire if using Grove connector).
   * @param[in] pin2 The pin controlling USB_OUTPUT2 (white wire if using Grove connector).
   *
   * @attention Ensure the pins are PWM-capable and correctly connected to the module.
   */
  UsbSwitch(uint8_t pin1, uint8_t pin2);

  /**
   * @brief Sets the output value using a PWM value.
   *
   * Writes a PWM value to the specified output(s), updating the internal status based on whether the value is
   * non-zero.
   *
   * @param[in] outputNo The output to control (1 for USB_OUTPUT1, 2 for USB_OUTPUT2, 0 for both).
   * @param[in] value The PWM value (0–255).
   *
   * @attention Ensure the value is within 0–255 and the output number is valid (0, 1, or 2).
   */
  void setOutputValue(uint8_t outputNo, int value);

  /**
   * @brief Sets the output value as a percentage.
   *
   * Converts a percentage to a PWM value and applies it to the specified output(s), updating the internal
   * status.
   *
   * @param[in] outputNo The output to control (1 for USB_OUTPUT1, 2 for USB_OUTPUT2, 0 for both).
   * @param[in] percentage The output value as a percentage (0–100%).
   *
   * @attention For some devices (e.g., pumping motors), a percentage below 59% may not activate the device.
   */
  void setOutputValuePercentage(uint8_t outputNo, int percentage);

  /**
   * @brief Retrieves the current output value.
   *
   * Reads the current PWM value of the specified output from the pin.
   *
   * @param[in] outputNo The output to query (1 for USB_OUTPUT1, 2 for USB_OUTPUT2).
   *
   * @attention Ensure the output number is valid (1 or 2).
   *
   * @return int The current PWM value (0–255), or 0 if an invalid output is specified.
   */
  int getOutputValue(uint8_t outputNo);

  /**
   * @brief Retrieves the current output value as a percentage.
   *
   * Converts the current PWM value of the specified output to a percentage.
   *
   * @param[in] outputNo The output to query (1 for USB_OUTPUT1, 2 for USB_OUTPUT2).
   *
   * @attention Ensure the output number is valid (1 or 2).
   *
   * @return int The current output value as a percentage (0–100%), or 0 if an invalid output is specified.
   */
  int getOutputValuePercentage(uint8_t outputNo);

  /**
   * @brief Toggles the output state between ON and OFF.
   *
   * Switches the specified output(s) between ON (PWM 255) and OFF (PWM 0), updating the internal status.
   *
   * @param[in] outputNo The output to toggle (1 for USB_OUTPUT1, 2 for USB_OUTPUT2, 0 for both).
   *
   * @attention Ensure the output number is valid (0, 1, or 2).
   */
  void toggleOutput(uint8_t outputNo);

  /**
   * @brief Checks the current status of the output.
   *
   * Returns whether the specified output is currently ON (non-zero PWM) or OFF.
   *
   * @param[in] outputNo The output to check (1 for USB_OUTPUT1, 2 for USB_OUTPUT2).
   *
   * @attention Ensure the output number is valid (1 or 2).
   *
   * @return bool `true` if the output is ON, `false` otherwise or if an invalid output is specified.
   */
  bool getStatus(uint8_t outputNo);

private:
  int  _pin1;     /** Pin of the first output (USB_OUTPUT1)*/
  int  _pin2;     /** Pin of the second output(USB_OUTPUT2)*/
  int  _value;    /** Value of the output */
  bool status[2]; /**< Status of the output (ON/OFF) */
};

#endif // USB_SWITCH_H

/* End of file -------------------------------------------------------- */