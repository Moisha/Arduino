#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <RTClib.h>
#include "wifi.h"
#include "readings.h"
#include "options.h"
#include "display.h"
#include "logging.h"

#define DHTTYPE DHT22     // DHT 22 (AM2302)

#define SOIL_PIN A0
#define LAMP_PIN D3
#define HUMIDIFIER_PIN D6
#define DHTPIN D5
#define WATER_PIN D4
#define GROW_BEG_SHITCH_PIN D7

DHT dht(DHTPIN, DHTTYPE); // temp and hum
RTC_DS1307 rtc; // real time clock

int lampRelayState = 0;
int lampMode = 0; // 0 - veg, 1 - bloom, 2 - on, 3 - off
int wateringState = 0;
int humidifierState = 0;

uint32_t wateringLastTime = 0;
uint32_t wateringStartTime = 0;

#define SECONDS_PER_HOUR 3600

void initRelays()
{
  pinMode(LAMP_PIN, OUTPUT);
  pinMode(WATER_PIN, OUTPUT);
  pinMode(HUMIDIFIER_PIN, OUTPUT);

  switchLamp(false);
  switchWatering(false);
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

  wateringLastTime = lastGoodTime;
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
  #ifdef LOG_TIME
    Serial.println("reading RTC");
  #endif

  DateTime dt = rtc.now();
  r->dt = dt.unixtime();

  #ifdef LOG_TIME
    Serial.println(dt.timestamp());
  #endif


  if (r->dt < readingsArchive[1]->dt || r->dt - readingsArchive[1]->dt > 600)
  {
    // цикл длился больше десяти минут, что вряд ли, или время пошло назад, что еще более вряд ли
    r->dt = readingsArchive[1]->dt; // чтобы в седующий проверять расхождение с тем же временем
    return false;
  }
  return true;
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
  lampMode = digitalRead(GROW_BEG_SHITCH_PIN); // read from switch
  #ifdef LOG_LAMP_MODE  
    Serial.print("lampMode ");
    Serial.println(lampMode);    
  #endif

  DateTime dt(r->dt);
  switchLamp(dt.hour() >= lampDayStartHour[lampMode] && dt.hour() < lampNightStartHour[lampMode]);
}

void switchWatering(bool v)
{
  logWatering("switchWatering ", false);
  logWatering(v);

  wateringState = v;
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

bool dayIsGoodForWatering(Readings *r)
{
  DateTime dtNow(r->dt);
  DateTime nowDate(dtNow.year(), dtNow.month(), dtNow.day());
  DateTime base(2024, 2, 9); // опорная дата для полива. Полили 09.02.2024, потом поливаем через указанное количество дней
  TimeSpan diff = nowDate - base;
  return diff.days() % daysBetweenWaterings == 0;
}

bool timeIsGoodForWatering(Readings *r)
{
  // Поливать будем сразу после выключения света
  DateTime dtNow(r->dt);
  return dtNow.hour() == lampNightStartHour[lampMode] && dtNow.minute() < 30;
}

bool wasWateringToday(Readings *r)
{
  DateTime dtWateringLastTime(wateringLastTime); 
  logWatering("wateringLastTime = ", false);
  logWatering(dtWateringLastTime.timestamp());

  int secondsFromLastWatering = r->dt - wateringLastTime;
  logWatering("secondsFromLastWatering = ", false);
  logWatering(secondsFromLastWatering);
  int minSecondsBetweenWaterings = SECONDS_PER_HOUR; // интервал начала полива полчаса, так что возьмем с запасом
  return secondsFromLastWatering < minSecondsBetweenWaterings;
}

void checkStartWatering(Readings *r)
{
  logWatering("checkStartWatering");

  if (lampRelayState) // watering at night time
  {
    logWatering("reason: lamp is on");
    return;
  }

  if (!dayIsGoodForWatering(r))
  {
    logWatering("reason: day is wrong ");
    return;  
  }

  if (!timeIsGoodForWatering(r))
  {
    logWatering("reason: time is wrong");
    return;  
  }

  if (wasWateringToday(r)) 
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

void initGrowVerSwitch()
{
  pinMode(GROW_BEG_SHITCH_PIN, INPUT);
}

void updateGlobalVars(Readings *r)
{
  r->lampRelayState = lampRelayState;
  r->wateringState = wateringState;
  r->lampMode = lampMode;
  r->wateringLastTime = wateringLastTime;
  r->humidifierState = humidifierState;
}

void setup() {
  delay(1000);  

  initReadings();
  initSerial(); 
  initDHT();
  initRTC(); 
  initDisplay();
  initRelays();
  initGrowVerSwitch();
  initWiFi();  
}

void loop()
{  
  Serial.print("Sketch DT: ");
  Serial.print(F(__DATE__));
  Serial.print(" ");
  Serial.println(F(__TIME__));

  Readings *r = prepareReadings();
  // scanWiFi();

  if (readRTC(r))
  {
    readDHT(r);
    readSoil(r);

    checkLampMode(r);
    checkWatering(r);
    checkHumidifier(r);
    
    updateGlobalVars(r);

    displayValues(r);

    postData(r);
  /*
    int humidifierStateTmp = humidifierState;
    if (wifi_needSend(r))
    {
      // увлажнитель - злейший враг WiFi и АЦП. Поэтому выключим его и перечитаем значение из SOIL_PIN
      switchHumidifier(false);
      delay(3000);
      readSoil(r);
      delay(1000);
      bool res = postData(r);
      switchHumidifier(humidifierStateTmp);
      if (!res && humidifierStateTmp)
        delay(30 * 1000); // пусть поработает увлажнитель, пока мы не ушли на следующий цикл, где опять не факт, что соединимся  
    }
    */
  }

  delay(3000);
}
