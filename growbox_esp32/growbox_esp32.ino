#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <RTClib.h>
#include "growbox_wifi.h"
#include "readings.h"
#include "options.h"
#include "display.h"
#include "logging.h"
#include <esp32-hal-log.h>

#define DHTTYPE DHT22     // DHT 22 (AM2302)

#define SOIL_PIN A0
#define DHT_PIN T0
#define LAMP_PIN T1
#define WATER_PIN T2
#define GROW_BLOOM_SHITCH_PIN T3
#define HUMIDIFIER_PIN T4
#define FAN_PIN T5

DHT dht(DHT_PIN, DHTTYPE); // temp and hum
RTC_DS1307 rtc; // real time clock

int lampRelayState = 0;
int lampMode = 0; // 0 - veg, 1 - bloom, 2 - on, 3 - off
int wateringState = 0;
int humidifierState = 0;
int fanState = 0;

uint32_t wateringLastTime = 0;
uint32_t wateringStartTime = 0;

void initRelays()
{
  pinMode(LAMP_PIN, OUTPUT);
  pinMode(WATER_PIN, OUTPUT);
  pinMode(HUMIDIFIER_PIN, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);

  switchLamp(false);
  switchWatering(false);
  switchHimidifier(true);
  switchFan(false);

  Serial.println("initRelays done");
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
}

void readDHT(Readings *r)
{
  float newT = dht.readTemperature();
  // if temperature read failed, don't change t value
  if (isnan(newT)) {
    Serial.println("Failed to read from DHT sensor!");
  }
  else {
    r->temperature = newT;
    logDHT("T=", false);
    logDHT(r->temperature);
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
}

void readRTC(Readings *r)
{
  #ifdef LOG_TIME
    Serial.println("reading RTC");
  #endif

  DateTime dt = rtc.now();
  r->dt = dt.unixtime();

  #ifdef LOG_TIME
    Serial.println(dt.timestamp());
  #endif
}

void readSoil(Readings *r)
{
  r->setSoilHumidityRaw(analogRead(SOIL_PIN)); // Читаем сырые данные с датчика,

  logDHT("Soil Hymidity ", false);
  logDHT((int)round(r->soilHumidityRaw), false);
  logDHT(" / ", false);
  logDHT((int)round(r->soilHumidity));
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

void switchLamp(bool v)
{
  #ifdef LOG_LAMP_MODE  
    Serial.print("switchLamp ");
    Serial.println(v);    
  #endif

  lampRelayState = v;
  digitalWrite(LAMP_PIN, v ? LOW : HIGH);  
}

void checkLampMode(Readings *r)
{
  DateTime dt(r->dt);

  lampMode = digitalRead(GROW_BLOOM_SHITCH_PIN); // read from switch
  switchLamp(dt.hour() >= lampDayStartHour[lampMode] && dt.hour() < lampNightStartHour[lampMode]);

  #ifdef LOG_LAMP_MODE  
    Serial.print("lampMode ");
    Serial.println(lampMode);    
  #endif
}

void switchWatering(bool v)
{
  logWatering("switchWatering ", false);
  logWatering(v);

  wateringState = v;
  digitalWrite(WATER_PIN, v ? LOW : HIGH);  
}

void switchHimidifier(bool v)
{
  logWatering("switchHimidifier ", false);
  logWatering(v);

  humidifierState = v;
  digitalWrite(WATER_PIN, v ? LOW : HIGH);  
}

void switchFan(bool v)
{
  logWatering("switchFan ", false);
  logWatering(v);

  fanState = v;
  digitalWrite(WATER_PIN, v ? LOW : HIGH);  
}

void checkStopWatering(Readings *r)
{
  logWatering("checkStopWatering");

  int wateringDurationSeconds = r->dt - wateringStartTime;
  int wateringDurationLimitSeconds = dailyWateringVolume / wateringPerMinute * 60;

  logWatering("wateringDurationSeconds = ", false);
  logWatering(wateringDurationSeconds);

  logWatering("wateringDurationLimitSeconds = ", false);
  logWatering(wateringDurationLimitSeconds);

  if (wateringDurationSeconds >= wateringDurationLimitSeconds)
  {
    switchWatering(false);
    wateringStartTime = 0;
    wateringLastTime = r->dt;    
  }  
}

void checkStartWatering(Readings *r)
{
  logWatering("checkStartWatering");

  if (lampRelayState) // watering at night time
  {
    logWatering("reason: lamp is on");
    return;
  }

  DateTime dtWateringLastTime(wateringLastTime); 
  logWatering("wateringLastTime = ", false);
  logWatering(dtWateringLastTime.timestamp());

  int secondsFromLastWatering = r->dt - wateringLastTime;
  logWatering("secondsFromLastWatering = ", false);
  logWatering(secondsFromLastWatering);
  // полив раз в сутки, а больше 13 часов ночь не должна вроде быть
  // Но и близкое к суткам время мы не можем указать, вдруг посреди ночи отключался свет и полив был не сразу после выключения лампы
  if (secondsFromLastWatering < 13 * 3600) 
  {
    logWatering("reason: interval not passed");
    return;
  }

  switchWatering(true);
  wateringStartTime = r->dt;
}

void checkWatering(Readings *r)
{
  logWatering("wateringState = ", false);
  logWatering(wateringState);

  if (wateringState)
    checkStopWatering(r);
  else
    checkStartWatering(r);
}

void initLampModeSwitch()
{
  pinMode(GROW_BLOOM_SHITCH_PIN, INPUT);
  Serial.println("Init Lamp Mode switch done");    
}

void updateGlobalVars(Readings *r)
{
  r->lampRelayState = lampRelayState;
  r->wateringState = wateringState;
  r->lampMode = lampMode;
  r->wateringLastTime = wateringLastTime;
}

void initI2C()
{
  Wire.begin(GROWBOX_SDA, GROWBOX_SCL);
}

void setup() {
  delay(1000);  

  initReadings();
  initI2C();
  initSerial(); 
  initDHT();
  initDisplay();
  initRTC(); 
  initRelays();
  initLampModeSwitch();
  initWiFi();  
}

void loop()
{
  Serial.println("go");
  // scanWiFi();

  Readings *r = prepareReadings();

  readRTC(r);
  readDHT(r);
  readSoil(r);

  checkLampMode(r);
  checkWatering(r);
  
  updateGlobalVars(r);

  displayValues(r);
  postData(r);

  delay(3000);
  yield();
}
