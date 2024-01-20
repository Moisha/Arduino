#ifndef growbox_logging_h
#define growbox_logging_h

#include <Arduino.h>
#include "options.h"

void logWatering(String s, bool ln = true) 
{
  #ifdef LOG_WATER
    Serial.print(s);  
    if (ln)
      Serial.println();  
  #endif
}

void logWatering(int n, bool ln = true)
{
  #ifdef LOG_WATER
    Serial.print(n);  
    if (ln)
      Serial.println();  
  #endif
}

void logDHT(String s, bool ln = true) 
{
  #ifdef LOG_DHT
    Serial.print(s);  
    if (ln)
      Serial.println();  
  #endif
}

void logDHT(int val, bool ln = true)
{
  #ifdef LOG_DHT
    Serial.print(val);  
    if (ln)
      Serial.println();  
  #endif
}

void logDHT(float val, bool ln = true)
{
  #ifdef LOG_DHT
    Serial.print(val);  
    if (ln)
      Serial.println();  
  #endif
}

#endif