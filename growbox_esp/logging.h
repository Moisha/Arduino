#ifndef growbox_logging_h
#define growbox_logging_h

#include <Arduino.h>
#include "options.h"

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

void logTime(int val, bool ln = true)
{
  #ifdef LOG_TIME
    Serial.print(val);  
    if (ln)
      Serial.println();  
  #endif
}

void logTime(String val, bool ln = true)
{
  #ifdef LOG_TIME
    Serial.print(val);  
    if (ln)
      Serial.println();  
  #endif
}

void LogWiFi(String s, bool ln = true)
{
  #ifdef LOG_HTTP
    Serial.print(s);  
    if (ln)
      Serial.println();  
  #endif
}

void LogWiFi(int val, bool ln = true)
{
  #ifdef LOG_HTTP
    Serial.print(val);  
    if (ln)
      Serial.println();  
  #endif
}

void logCommon(String s, bool ln = true) 
{
  #ifdef LOG_COMMON
    Serial.print(s);  
    if (ln)
      Serial.println();  
  #endif
}

void logCommon(int val, bool ln = true)
{
  #ifdef LOG_COMMON
    Serial.print(val);  
    if (ln)
      Serial.println();  
  #endif
}

void logLamp(String s, bool ln = true) 
{
  #ifdef LOG_LAMP_MODE
    Serial.print(s);  
    if (ln)
      Serial.println();  
  #endif
}

void logLamp(int val, bool ln = true)
{
  #ifdef LOG_LAMP_MODE
    Serial.print(val);  
    if (ln)
      Serial.println();  
  #endif
}

#endif