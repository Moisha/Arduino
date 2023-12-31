#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <RTClib.h>
#include <LiquidCrystal_I2C.h>

#define RTC_CONNECTED 
#define RTC_UPDATE_TIME 

#define LOG_TIME
#define LOG_DHT
#define LOG_WATER
#define LOG_LAMP_MODE

#define DHTTYPE DHT22     // DHT 22 (AM2302)
#define DHTPIN D5

#define SOIL_PIN A0
#define LAMP_PIN D3
#define WATER_PIN D6
#define GROW_BEG_SHITCH_PIN D7

#define READINGS_ARCHIVE_LENGTH 100
#define LCD_FILLER "                "

DHT dht(DHTPIN, DHTTYPE); // temp and hum
RTC_DS1307 rtc; // real time clock
LiquidCrystal_I2C lcd(0x27, 16, 2); // display

int displayMode = 0;

int lampMode = 0; // 0 - grow, 1 - veg
int lampNightStartHour[2] = {22, 20};
int lampDayStartHour[2] = {3, 8};
bool lampRelayState = false;

const float dailyWateringVolume = 100;
const float wateringPerMinute = 125;

uint32_t wateringLastTime = 0;
uint32_t wateringStartTime = 0;
bool wateringState = false;

class Readings
{
  public:
    uint32_t dt;
    float temperature;
    float humidity;
    float soilHumidity;

  Readings() 
  { 
      init();
  };

  void init()
  {
    dt = 0;
    temperature = NAN;
    humidity = NAN;
    soilHumidity = NAN;
  }

  void assign(Readings *source)
  {
    dt = source->dt;
    temperature = source->temperature;
    humidity = source->humidity;
    soilHumidity = source->soilHumidity;    
  }
};

Readings *readingsArchive[READINGS_ARCHIVE_LENGTH];

void initRelays()
{
  pinMode(LAMP_PIN, OUTPUT);
  pinMode(WATER_PIN, OUTPUT);

  switchLamp(false);
  switchWatering(false);

  Serial.println("initRelays done");
}

void initDisplay()
{
  Wire.begin(D2, D1);
  lcd.begin(D2, D1);

  lcd.clear();         
  lcd.backlight();      // Make sure backlight is on  

  lcd.setCursor(0, 0);  
}

void initRTC()
{
  #ifdef RTC_CONNECTED
    Serial.println("initRTC");
    if (!rtc.begin()) 
      Serial.println("Couldn't find RTC");
    else
      Serial.println("RTC found");

    #ifdef RTC_UPDATE_TIME
      // sets the RTC to the date & time on PC this sketch was compiled
      rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    #endif
  #endif
}

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

void readDHT(Readings *r)
{
  float newT = dht.readTemperature();
  // if temperature read failed, don't change t value
  if (isnan(newT)) {
    Serial.println("Failed to read from DHT sensor!");
  }
  else {
    r->temperature = newT;
    #ifdef LOG_DHT
      Serial.print("T=");
      Serial.println(r->temperature);
    #endif
  }

  // Read Humidity
  float newH = dht.readHumidity();
  // if humidity read failed, don't change h value 
  if (isnan(newH)) 
  {
    Serial.println("Failed to read from DHT sensor!");
  }
  else 
  {
      r->humidity = newH;
    #ifdef LOG_DHT
      Serial.print("H=");
      Serial.println(r->humidity);
    #endif
  }
}

void readRTC(Readings *r)
{
  #ifdef RTC_CONNECTED
    #ifdef LOG_TIME
      Serial.println("reading RTC");
    #endif

    DateTime dt = rtc.now();
    r->dt = dt.unixtime();

    #ifdef LOG_TIME
      Serial.println(dt.timestamp());
    #endif
  #endif
}

void readSoil(Readings *r)
{
  int soilHumiditiSensorMin = 960;                    // Определяем минимальное показание датчика (в воздухе),
  int soilHumiditiSensorMax = 655;      

  int sh = analogRead(SOIL_PIN);             // Читаем сырые данные с датчика,
  r->soilHumidity = map(sh, soilHumiditiSensorMin, soilHumiditiSensorMax, 0, 100);  // адаптируем значения от 0 до 100
  #ifdef LOG_DHT
    Serial.printf("Soil Hymidity %d / %f", sh, r->soilHumidity);
    Serial.println(r->humidity);
  #endif  
}

void initDHT()
{
  Serial.println("initDHT");
  dht.begin();
  Serial.println("initDHT done");
}

void initSerial()
{
  Serial.begin(115200); 
  Serial.println("Startup");
}

void initReadings()
{ 
  for (int i = 0; i < READINGS_ARCHIVE_LENGTH; i++)
    readingsArchive[i] = new Readings();
}

Readings* prepareReadings()
{
  for (int i = READINGS_ARCHIVE_LENGTH - 1; i >= 1 ; i--)  
    readingsArchive[i]->assign(readingsArchive[i - 1]);

  Readings *r = readingsArchive[0];  
  r->init();

  return r;
}

void lcdFirstLine()
{
  lcd.setCursor(0, 0);    
}

void lcdSecondLine()
{
  lcd.setCursor(0, 1);    
}

void displayDt(Readings *r)
{
  DateTime dt(r->dt);

  lcdFirstLine();  
  lcd.print(dt.timestamp(DateTime::TIMESTAMP_TIME));    
  lcd.print(" R:");
  lcd.print(rtc.isrunning());    
  lcd.print(LCD_FILLER);

  lcdSecondLine();  
  lcd.printf("Mem: ");
  lcd.print(ESP.getFreeHeap());
  lcd.print(LCD_FILLER);
}

void displayTH(Readings *r)
{
  lcdFirstLine();  
  lcd.printf("T: ");
  lcd.print(r->temperature);    
  lcd.print(LCD_FILLER);

  lcdSecondLine();  
  lcd.printf("H: ");
  lcd.print(r->humidity);
  lcd.print(LCD_FILLER);  
}

void displaySoil(Readings *r)
{
  lcdFirstLine();  
  lcd.print("Soil H: ");
  if (isnan(r->soilHumidity))
    lcd.print("---");
  else
    lcd.print(r->soilHumidity);

  lcd.print(LCD_FILLER);

  DateTime lastWatering(wateringLastTime);
  lcdSecondLine();  
  lcd.print("W: ");
  lcd.printf("%02d.%02d %02d:%02d", lastWatering.day(), lastWatering.month(), lastWatering.hour(), lastWatering.minute());
  lcd.print(LCD_FILLER);
}

void displayLamp(Readings *r)
{
  lcdFirstLine();  
  lcd.print("M: ");
  lcd.print(lampMode ? "Veg" : "Grow");
  lcd.print(", R: ");
  lcd.print(lampRelayState);
  lcd.print(LCD_FILLER);

  lcdSecondLine();  
  lcd.print("D: ");
  lcd.print(lampDayStartHour[lampMode]);
  lcd.print(", N: ");
  lcd.print(lampNightStartHour[lampMode]);
  lcd.print(LCD_FILLER);  
}

void displayValues(Readings *r)
{
  switch (displayMode)
  {
    case 0:
      displayDt(r);
      break;

    case 1:
      displayTH(r);
      break;

    case 2:
      displayLamp(r);
      break;
    
    case 3:
      displaySoil(r);
      break;

    default:
      break;
  }

  displayMode++;
  displayMode %= 4;
}

void switchLamp(bool v)
{
  lampRelayState = v;

  #ifdef LOG_LAMP_MODE  
    Serial.print("switchLamp ");
    Serial.println(lampRelayState);    
  #endif

  digitalWrite(LAMP_PIN, lampRelayState ? LOW : HIGH);  
}

void checkLampMode(Readings *r)
{
  DateTime dt(r->dt);

  lampMode = digitalRead(GROW_BEG_SHITCH_PIN); // read from switch
  switchLamp(dt.hour() >= lampDayStartHour[lampMode] && dt.hour() < lampNightStartHour[lampMode]);

  #ifdef LOG_LAMP_MODE  
    Serial.print("lampMode ");
    Serial.println(lampMode);    
  #endif
}

void switchWatering(bool v)
{
  wateringState = v;

  logWatering("switchWatering ", false);
  logWatering(wateringState);

  digitalWrite(WATER_PIN, wateringState ? LOW : HIGH);  
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
  if (wateringState)
    checkStopWatering(r);
  else
    checkStartWatering(r);
}

void initGrowVerSwitch()
{
  pinMode(GROW_BEG_SHITCH_PIN, INPUT);
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
}

void loop()
{
  Serial.println("go");

  Readings *r = prepareReadings();

  readDHT(r);
  readRTC(r);
  readSoil(r);

  checkLampMode(r);
  checkWatering(r);
  
  displayValues(r);

  delay(3000);
  yield();
}
