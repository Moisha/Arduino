#pragma once

#include "devices.h"
#include "options.h"
#include "readings.h"

void readDHT(Readings *r)
{
  float newT = dht.readTemperature();
  // if temperature read failed, don't change t value
  if (isnan(newT)) {
    Serial.println("Failed to read from DHT sensor!");
  }
  else {
    logDHT("T=", false);
    logDHT(newT);
      
    r->temperature = newT;
  }

  // Read Humidity
  float newH = dht.readHumidity();
  // if humidity read failed, don't change h value 
  if (isnan(newH)) 
  {
    logDHT("Failed to read from DHT sensor!");
  }
  else 
  {
      r->humidity = newH;
      logDHT("H=", false);
      logDHT(r->humidity);
  }

  if (isnan(newT) || isnan(newH) || newT < 10 || newT > 50 || newH < 0 || newH > 100)
  {
    r->temperature = NAN;
    r->humidity = NAN;
    logDHT("Wrong temperature or humidity");
    return;
  }  
}

bool readRTC(Readings *r)
{
  DateTime dt = rtc.now();
  r->dt = dt.unixtime();

  logTime("reading RTC: ", false);
  logTime(dt.timestamp());

  // Время от NTP-сервера считаем более корректным, чем глюкавый RTC
  long wifiDT = getNTPTime();
  if (wifiDT > 0)
  {    
    // если разошлись больше чем на 10 минут, поправим RTC
    if (abs(wifiDT - (long)r->dt) > 600)
      rtc.adjust(DateTime(wifiDT));

    r->dt = wifiDT;
    return true;
  }

  if (r->dt < readingsArchive[1]->dt || r->dt - readingsArchive[1]->dt > 600)
  {
    // цикл длился больше десяти минут, что вряд ли, или время пошло назад, что еще более вряд ли
    r->dt = readingsArchive[1]->dt; // чтобы в седующий проверять расхождение с тем же временем
    return false;
  }
  return true;
}

void switchHumidifier(bool v)
{
  logDHT("switchHumidifier ", false);
  logDHT(v);

  humidifierState = v;
  digitalWrite(HUMIDIFIER_PIN, v ? LOW : HIGH);  
}

void switchLamp(bool v)
{
  #ifdef LOG_LAMP_MODE  
    Serial.print("switchLamp ");
    Serial.println(v);    
  #endif

  lampRelayState = v;
  digitalWrite(LAMP_PIN, v ? LOW : HIGH);  
}

void checkHumidifier(Readings *r)
{
  if (!isnan(r->humidity))
  {
    if (r->humidity < MIN_HUMIDITY)
    {
      switchHumidifier(true);
      return;
    }
    else
    if (r->humidity >= MAX_HUMIDITY)
    {
      switchHumidifier(false);
      return;
    }
  }

  logDHT("humidifier ", false);
  logDHT(humidifierState);
}

void checkLampMode(Readings *r)
{
  lampMode = digitalRead(GROW_VEG_SWITCH_PIN); // read from switch
  #ifdef LOG_LAMP_MODE  
    Serial.print("lampMode ");
    Serial.println(lampMode);    
  #endif

  DateTime dt(r->dt);
  int dayStart = lampDayStartHour[lampMode];
  int nightStart = lampNightStartHour[lampMode];
  if (dayStart < nightStart)
    switchLamp(dt.hour() >= dayStart && dt.hour() < nightStart);
  else
    switchLamp(dt.hour() < nightStart || dt.hour() >= dayStart);
}

void updateGlobalVars(Readings *r)
{
  r->lampRelayState = lampRelayState;
  r->lampMode = lampMode;
  r->humidifierState = humidifierState;
}