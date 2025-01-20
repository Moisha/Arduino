#ifndef growbox_wifi_h
#define growbox_wifi_h

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include "options.h"
#include "readings.h"
#include "logging.h"
#include <NTPClient.h>
#include <WiFiUdp.h>

Readings lastWiFiExchangeReadings;
WiFiClient wifiClient; 

// 5 hours - time offset in seconds
// 3 minutes - request frequency in milliseconds
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP,"pool.ntp.org", 3600 * 5, 3 * 60 * 1000);
  
void initWiFi()
{
  WiFi.mode(WIFI_STA);
}

bool connectWiFi()
{
  LogWiFi("Try connecting to WiFi ", false);
  LogWiFi(WIFI_SSID);
  if (WiFi.status() == WL_CONNECTED)
  {
    LogWiFi("WiFi connected");
    return true;
  }
  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int tries = 40; // 20 sec 
  while (--tries && WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    LogWiFi(".", false);
  }

  wl_status_t st = WiFi.status();
  if (st != WL_CONNECTED)
  {
    LogWiFi("Non Connecting to WiFi..", false);
    LogWiFi((int)st);
    return false;
  }

  LogWiFi("WiFi connected");

  timeClient.begin();
  return true;
}

bool checkHttpResponceCode(HTTPClient &http, int httpCode)
{
 // httpCode will be negative on error
  if (httpCode <= 0)
  {
    LogWiFi("[HTTP] POST... failed, error: ", false);
    LogWiFi(http.errorToString(httpCode));
    return false;
  }  
  
  // HTTP header has been send and Server response header has been handled
  LogWiFi("[HTTP] POST... code:", false);
  LogWiFi(String(httpCode));

  return httpCode == HTTP_CODE_OK;
}

void logHttpResponce(HTTPClient &http)
{
  const String& payload = http.getString();
  LogWiFi("received payload:<<");
  LogWiFi(payload);
  LogWiFi(">>");
}

bool doPostMeasurings(String url, String json)
{
  LogWiFi("URL: ", false); 
  LogWiFi(url);

  HTTPClient http;
  http.begin(wifiClient, url);
  http.addHeader("Content-Type", "application/json");

  int httpCode = http.POST(json);
  bool res = checkHttpResponceCode(http, httpCode);
  if (res) 
    logHttpResponce(http);

  http.end();
  return res;
}

bool postMeasurings(Readings *r)
{
  String json = r->toJSON();
  LogWiFi(json);

  doPostMeasurings(WIFI_DIAGNOSTIC_DATA_URL, json);
  return doPostMeasurings(WIFI_POST_DATA_URL, json);
}

bool wifi_needSend(Readings *r)
{
  bool res = r->dt - lastWiFiExchangeReadings.dt >= ARCHIVE_TIME_SECONDS
    || (r->lampMode != lastWiFiExchangeReadings.lampMode)
    || (r->lampRelayState != lastWiFiExchangeReadings.lampRelayState)
    || (r->wateringState != lastWiFiExchangeReadings.wateringState)
    || (r->humidifierState != lastWiFiExchangeReadings.humidifierState);

  LogWiFi("WiFi: ", false);
  LogWiFi(res ? "Need send" : "No need send");

  return res;
}

bool postData(Readings *r)
{
  LogWiFi("WiFi: post data");
  if (!wifi_needSend(r))
    return false;

  if (!connectWiFi())
    return false;

  if (!postMeasurings(r))
    return false;

  lastWiFiExchangeReadings.assign(r);
  return true;
}

unsigned long getNTPTime()
{
  logTime("reading NTP: ", false);
  logTime(timeClient.update() ? 1 : 0, false);
  logTime(", ", false);
  
  if (!timeClient.isTimeSet())  
  {
    logTime("time not set");
    return 0;
  }  
  
  DateTime dt (timeClient.getEpochTime());
  logTime(dt.timestamp());
  return timeClient.getEpochTime();
}

void scanWiFi()
{
  Serial.println("Wifi scan started");

  // WiFi.scanNetworks will return the number of networks found
  int n = WiFi.scanNetworks();
  Serial.println("Wifi scan ended");
  if (n == 0) {
    Serial.println("no networks found");
  } else {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i) {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(") ");
      Serial.print(WiFi.SSID(i));// SSID

                  
      Serial.print(WiFi.RSSI(i));//Signal strength in dBm  
      Serial.print("dBm (");
      
      Serial.print(WiFi.RSSI(i));//Signal strength in %  
     Serial.print("% )"); 
      if(WiFi.encryptionType(i) == ENC_TYPE_NONE)
      {
          Serial.println(" <<***OPEN***>>");        
      }else{
          Serial.println();        
      }

      delay(10);
    }
  }
  Serial.println("");
}



#endif