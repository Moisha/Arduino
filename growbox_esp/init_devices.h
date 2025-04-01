#pragma once

#include "devices.h"
#include "options.h"
#include "handle_devices.h"

void initRelays()
{
  pinMode(LAMP_PIN, OUTPUT);
  pinMode(HUMIDIFIER_PIN, OUTPUT);

  switchLamp(false);
  switchHumidifier(true);

  Serial.println("initRelays done");
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
  Serial.println("initRTC");
  if (!rtc.begin()) 
  {
    Serial.println("Couldn't find RTC");
    return;
  }

  Serial.println("RTC found");

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
  pinMode(GROW_VEG_SWITCH_PIN, INPUT);
}

void printGreeting()
{
  Serial.print("Sketch DT: ");
  Serial.print(F(__DATE__));
  Serial.print(" ");
  Serial.println(F(__TIME__));
}