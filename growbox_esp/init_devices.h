#pragma once

#include "devices.h"
#include "options.h"
#include "handle_devices.h"
#include <Wire.h>

void initRelays()
{
  logCommon("pinMode lamp"); 
  pinMode(LAMP_PIN, OUTPUT);

  logCommon("pinMode humidifier"); 
  pinMode(HUMIDIFIER_PIN, OUTPUT);

  logCommon("pinMode done"); 

  switchLamp(false);
  switchHumidifier(true);

  logCommon("initRelays done");
}

void checkInitialDT()
{
  int lastGoodTime = 0;
  // Время иногда возвращается загадочное. Поэтому поступим так. 
  // Пять раз подряд считаем время с RTC. Если вся пять чтений пройдут подряд, то считаем, 
  // что опорное время есть и от него оталкиваемся в верификации показаний RTC
  bool res = false;
  while (!res)
  {
    initReadings();
    res = true;
    for (int i = 0; i < 5; i++)
    {
      Readings *r = prepareReadings();
      r->dt = rtc.now().unixtime();
      lastGoodTime = r->dt;
      if (i > 0)
        if (r->dt < readingsArchive[1]->dt || r->dt - readingsArchive[1]->dt > 5)
        {
          res = false;
          break;
        }
      delay(1000);      
    }
  }
}

void initRTC()
{
  logCommon("RTC not configured");
  return;

  logCommon("initRTC");
  if (!rtc.begin()) 
  {
    logCommon("Couldn't find RTC");
    return;
  }

  logCommon("RTC found");

  #ifdef RTC_UPDATE_TIME
    // sets the RTC to the date & time on PC this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  #endif

  checkInitialDT();
}

void initDHT()
{
  logDHT("initDHT");
  dht.begin();
  logDHT("initDHT done");
}

void initSerial()
{
  Serial.begin(115200); 
  Serial.println("Startup");
}

void initGrowVerSwitch()
{
  pinMode(GROW_VEG_SWITCH_PIN, INPUT_PULLDOWN);
}

void printGreeting()
{
  Serial.print("Sketch DT: ");
  Serial.print(F(__DATE__));
  Serial.print(" ");
  Serial.println(F(__TIME__));
}

void initI2C()
{
  Wire.begin(SDA_PIN, SCL_PIN);
}

void initDisplay()
{
  logCommon("init display");
  lcd.begin(SDA_PIN, SCL_PIN);

  logCommon("clear display");

  lcd.clear();         
  lcd.backlight();      // Make sure backlight is on  

  lcd.setCursor(0, 0);  
}