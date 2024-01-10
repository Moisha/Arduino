#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Arduino.h>
#include "options.h"
#include "readings.h"

Readings lastWiFiExchangeReadings;

void LogWiFi(String s, bool ln = true)
{
  #ifdef LOG_HTTP
    Serial.print(s);  
    if (ln)
      Serial.println();  
  #endif
}

void initWiFi()
{
  WiFi.mode(WIFI_STA);
}

bool connectWiFi()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    LogWiFi("WiFi connected");
    return true;
  }

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int tries = 10; 
  while (--tries && WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    LogWiFi(".", false);
  }

  if (WiFi.status() != WL_CONNECTED)
  {
    LogWiFi("Non Connecting to WiFi..");
    return false;
  }

  LogWiFi("WiFi connected");
  LogWiFi("IP address: ", false);
  LogWiFi(WiFi.localIP());
  return true;
}

bool checkHttpResponceCode(int httpCode)
{
 // httpCode will be negative on error
  if (httpCode <= 0)
  {
    LogWiFi("[HTTP] POST... failed, error: ", false);
    LogWiFi(http.errorToString(httpCode).c_str());
    return false;
  }  
  
  // HTTP header has been send and Server response header has been handled
  LogWiFi("[HTTP] POST... code:", false);
  LogWiFi(httpCode);

  // file found at server
  if (httpCode == HTTP_CODE_OK) {
    const String& payload = http.getString();
    LogWiFi("received payload:<<");
    LogWiFi(payload);
    LogWiFi(">>");
    return true;
  }

  return false;
}

bool postMeasurings(Readings *r)
{
  WiFiClient wifiClient; 
  HTTPClient http;
  http.begin(wifiClient, WIFI_POST_DATA_URL);
  http.addHeader("Content-Type", "application/json");

  String json = r->toJSON();
  LogWiFi(json);

  int httpCode = http.POST(json);
  http.end();

  return checkHttpResponceCode(httpCode); 
}

bool needSend(Readings *r)
{
  return 
    r->dt - lastWiFiExchangeReadings.dt < ARCHIVE_TIME_SECONDS
    || r->lampMode != lastWiFiExchangeReadings.lampMode
    || r->lampRelayState != lastWiFiExchangeReadings.lampRelayState
    || r->wateringState != lastWiFiExchangeReadings.wateringState;
}

void postData(Readings *r)
{
  if (!needSend(r))
    return;

  if (!connectWiFi())
    return;

  if (!postMeasurings(r))
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