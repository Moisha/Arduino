#include <Adafruit_Sensor.h>
#include "wifi_client.h"
#include "readings.h"
#include "options.h"
#include "display.h"
#include "logging.h"
#include "devices.h"
#include "init_devices.h"
#include "handle_devices.h"

void setup() {
  delay(1000);  

  initReadings();
  initSerial(); 
  initDHT();
  initI2C();
  initDisplay();
  initRTC();
  initRelays();
  initGrowVerSwitch();
  initWiFi();  
}

void loop()
{  
  printGreeting();

  Readings *r = prepareReadings();
  // scanWiFi();

  if (readRTC(r))
  {
    readDHT(r);
    checkLampMode(r);
    checkHumidifier(r);
    updateGlobalVars(r);
    displayValues(r);
    postData(r);
  }

  delay(1000);
}
