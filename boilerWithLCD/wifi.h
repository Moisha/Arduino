#ifndef growbox_wifi_h
#define growbox_wifi_h

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

WiFiClient wifiClient; 
long long lastSendMillis = 0;

#define WIFI_SSID "Trifon"
#define WIFI_PASSWORD "18000dong"
#define WIFI_POST_DATA_URL "http://192.168.1.100:9000" // Melchior
#define ARCHIVE_TIME_SECONDS 60
#define DEVICE_LOCATION "Boiler"
 
void initWiFi()
{
  WiFi.mode(WIFI_STA);
} 

void LogWiFi(String msg, bool lineBreak = true)
{
    Serial.print(msg);
    if (lineBreak)
      Serial.println();
}

void LogWiFi(int msg, bool lineBreak = true)
{
    Serial.print(msg);
    if (lineBreak)
      Serial.println();
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

String formatForJSON(String name, String val)
{
  return "\"" + name + "\": \"" + val + "\",\n";
}

String floatToStr(float val)
{
  char buf[50];
  dtostrf(val, -1, 1, buf);
  return buf;
}

String toJSON(float temperature, int targetTemp, bool relayState)
{
  // Может показаться странным, что тут везде humidity, но на температуру нет ни задания, ни состояния реле, а вводить их специально не хочется
  String res = "{\r\n" +
    formatForJSON("source", DEVICE_LOCATION) +
    formatForJSON("humidifierState", String((int)relayState)) +
    formatForJSON("targetHumidity", String((int)targetTemp));

    if (!isnan(temperature))
      res += formatForJSON("humidity", floatToStr(temperature));
    
    res = res.substring(0, res.length() - 2) + "\n"; // уберем запятую в конце

    res += "}";

    return res;
}

bool postMeasurings(float temperature, int targetTemp, bool relayState)
{
  String json = toJSON(temperature, targetTemp, relayState);
  LogWiFi(json);

#ifdef WIFI_DIAGNOSTIC_DATA_URL  
  doPostMeasurings(WIFI_DIAGNOSTIC_DATA_URL, json);
#endif  
  return doPostMeasurings(WIFI_POST_DATA_URL, json);
}

bool wifi_needSend()
{
  long long currentMillis = millis();
  LogWiFi("millis = ", false);
  LogWiFi(currentMillis);
  LogWiFi("lastSendMillis = ", false);
  LogWiFi(lastSendMillis);
  
  bool res = abs(currentMillis - lastSendMillis) > ARCHIVE_TIME_SECONDS * 1000;

  LogWiFi("WiFi: ", false);
  LogWiFi(res ? "Need send" : "No need send");

  return res;
}

bool postData(int temperature, int targetTemp, bool relayState)
{
  LogWiFi("WiFi: post data");
  if (!wifi_needSend())
    return false;

  if (!connectWiFi())
    return false;

  if (!postMeasurings(temperature, targetTemp, relayState))
    return false;

  lastSendMillis = millis();
  return true;
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