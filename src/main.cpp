#include "main.h"

void setup()
{
  Serial.begin(9600);
  Wire.begin(SDA, SCL, 100000UL);

  esp_pm_config_esp32s3_t pm_config = {
  .max_freq_mhz       = 240,
  .min_freq_mhz       = 80,
  .light_sleep_enable = true,
  };
  esp_err_t retVal = esp_pm_configure(&pm_config);
  Serial.print("Power Management Init Status: ");
  Serial.println(esp_err_to_name(retVal));

// WiFi Setup
#ifdef WIFI_MODULE
  wifiSetup();
#endif

// Devices Setup
// Sensors
#ifdef SENSORS_MODULE
  sensorsTaskSetup();
#endif

// Actuators
#ifdef ACTUATORS_MODULE
  actuatorTaskSetup();
#endif

#ifdef LCD_MODULE
  lcdTaskSetup();
#endif

#ifdef BUTTON_MODULE
  buttonTaskSetup();
#endif

// IOT Server Setup
#ifdef IOT_SERVER_MODULE
  iotServerSetup();
#endif
}

void loop() {}
