#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Arduino.h>
#include "options.h"
#include "readings.h"

Readings lastWiFiExchangeReadings;

void initWiFi()
{
  // WiFi.mode(WIFI_STA);
}

bool connectWiFi()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("WiFi connected");
    return true;
  }

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int tries = 10; 
  while (--tries && WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Non Connecting to WiFi..");
    return false;
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  return true;
}

bool postMeasurings()
{
  /*
  HTTPClient http;
  http.begin(WiFi., WIFI_POST_DATA_URL);
  http.addHeader("Content-Type", "text/plain");
  */
  return true;
}

bool needSend(Readings *r)
{
  return r->dt - lastWiFiExchangeReadings.dt < ARCHIVE_TIME_SECONDS;
}

void postData(Readings *r)
{
  if (!needSend(r))
    return;

  if (!connectWiFi())
    return;

  if (!postMeasurings())
    return;

  lastWiFiExchangeReadings.assign(r);    
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