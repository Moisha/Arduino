// библиотека для работы с протоколом 1-Wire
#include <OneWire.h>
// библиотека для работы с датчиком DS18B20
#include <DallasTemperature.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include "wifi.h"

// если не прописывается программа - зачисти флешку
// esptool.py --port /dev/ttyUSB0 erase_flash

// сигнальный провод датчика температуры
#define ONE_WIRE_BUS D6

// Реле упраления
#define PIN_RELAY D5

//  реле Ардуино инверсное, SSR-10 прямое  
#define STRAIGTH_RELAY true

#define TEMP_HISTERESIS 5

const int LowTemp = 30;
const int HighTemp = 80;
const int LowSensor = 50;
const int HighSensor = 1024;

// создаём объект для работы с библиотекой OneWire
OneWire oneWire(ONE_WIRE_BUS);
 
// создадим объект для работы с библиотекой DallasTemperature
DallasTemperature sensor(&oneWire);
 
// display
LiquidCrystal_I2C lcd(0x27, 16, 2); 

bool RelayState = false;

int indicator = 0;

void setRelay(bool state)
{
  Serial.print("Relay: ");
  Serial.println(state);

  bool command = state;
  if (STRAIGTH_RELAY)
    command = !command;

  if (command)
    digitalWrite(PIN_RELAY, LOW);
  else
    digitalWrite(PIN_RELAY, HIGH);

  RelayState = state;
}

int resistanceToTemp(int sensorValue)
{
  if (sensorValue <= 50)
    return 15; // если даже выключено или датчик отпал, все равно не даем заморозить батареи

  // 50 - это 30 градусов, 1024 - 80
  float res;
  res = (sensorValue - LowSensor);
  res = res / (HighSensor - LowSensor) * (HighTemp - LowTemp) + LowTemp;
  return res;
}

void checkRelay(float temperature, int targetTemp)
{
  if (isnan(temperature) || isnan(targetTemp))
  {
    setRelay(false);
    return;
  }

  if (temperature <= (targetTemp - TEMP_HISTERESIS))
  {
    setRelay(true);
  }
  else
  if (temperature >= (targetTemp + TEMP_HISTERESIS))
  {
    setRelay(false); 
  }
}

void lcdFirstLine()
{
  lcd.setCursor(0, 0);    
}

void lcdSecondLine()
{
  lcd.setCursor(0, 1);    
}

char nextIndicator()
{
  indicator++;
  indicator %= 4;

  switch (indicator)
  {
    case 0: return '/';
    case 1: return '-';
    case 2: return char(0);
    case 3: return '|';
  }

  return '0';
}

void displayState(int temperature, int targetTemp)
{
  lcdFirstLine();
  lcd.printf("T: %02d (%02d - %02d)           ", temperature, targetTemp - TEMP_HISTERESIS, targetTemp + TEMP_HISTERESIS);
  lcdSecondLine();
  lcd.printf("R%d             ", RelayState);
  lcd.print(nextIndicator());
}

void setup(){
  // инициализируем работу Serial-порта
  Serial.begin(115200);
  // начинаем работу с датчиком
  sensor.begin();
  // устанавливаем разрешение датчика от 9 до 12 бит
  sensor.setResolution(12);
  

  Wire.begin(D2, D1);
  
  lcd.begin(D2, D1);
  lcd.clear();         
  lcd.backlight();

  // вместо обратного слеша на 1602 значой йены, запилим свой
  byte ReverseSlash[] = {
    B00000,
    B10000,
    B01000,
    B00100,
    B00010,
    B00001,
    B00000,
    B00000
  };

  lcd.createChar(0, ReverseSlash);

  setRelay(false);

  pinMode(A0, INPUT); 
  pinMode(PIN_RELAY, OUTPUT);
  initWiFi();
}

void loop(){
  // переменная для хранения температуры
  int temperature;
  // отправляем запрос на измерение температуры
  sensor.requestTemperatures();
  // считываем данные из регистра датчика
  temperature = sensor.getTempCByIndex(0);
  // выводим температуру в Serial-порт
  Serial.print("Temp C: ");
  Serial.println(temperature);

  int r = analogRead(A0);
  Serial.print("Resistance: ");
  Serial.println(r);

  int targetTemp = resistanceToTemp(r);
  Serial.print("Target Temp: ");
  Serial.println(targetTemp);
  
  checkRelay(temperature, targetTemp);
  displayState(temperature, targetTemp);
  postData(temperature, targetTemp, RelayState);

  delay(1000);
}
