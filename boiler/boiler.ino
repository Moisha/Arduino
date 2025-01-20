// библиотека для работы с протоколом 1-Wire
#include <OneWire.h>
// библиотека для работы с датчиком DS18B20
#include <DallasTemperature.h>

// сигнальный провод датчика
#define ONE_WIRE_BUS 14

#define PIN_RELAY 12
#define TEMP_HISTERESIS 5

#define MIN_COMPRESSOR_SECONDS_OFF 300
#define MIN_COMPRESSOR_SECONDS_ON 60

const int LowTemp = 30;
const int HighTemp = 80;
const int LowSensor = 50;
const int HighSensor = 1024;

// создаём объект для работы с библиотекой OneWire
OneWire oneWire(ONE_WIRE_BUS);
 
// создадим объект для работы с библиотекой DallasTemperature
DallasTemperature sensor(&oneWire);
 
bool RelayState;

void setRelay(bool state)
{
  Serial.print("Relay: ");
  Serial.println(state);

  if (state)
    digitalWrite(PIN_RELAY, LOW);
  else
    digitalWrite(PIN_RELAY, HIGH);

  RelayState = state;
}


void setup(){
  // инициализируем работу Serial-порта
  Serial.begin(9600);
  // начинаем работу с датчиком
  sensor.begin();
  // устанавливаем разрешение датчика от 9 до 12 бит
  sensor.setResolution(12);

  setRelay(false);

  pinMode(A0, INPUT); 
  pinMode(PIN_RELAY, OUTPUT);
  pinMode(D4, OUTPUT); 
}

void blink(int prefix, int val)
{
  achtung(prefix);
  delay(500);

  for (int i = 0; i < val / 10; i++)
  {
    digitalWrite(D4, LOW);
    delay(500);
    digitalWrite(D4, HIGH);
    delay(500);
  }

  delay(500);

  for (int i = 0; i < val % 10; i++)
  {
    digitalWrite(D4, LOW);
    delay(200);
    digitalWrite(D4, HIGH);
    delay(200);
  }
}

void achtung(int val)
{
  for (int i = 0; i < val; i++)
  {    
    digitalWrite(D4, LOW);
    delay(100);
    digitalWrite(D4, HIGH);
    delay(100);
  }
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

  blink(3, temperature);
  delay(1000);

  blink(5, targetTemp);
  delay(1000);
}
