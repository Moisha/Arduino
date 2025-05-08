#pragma once

#include <DHT.h>
#include <RTClib.h>
#include "options.h"
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include "MQ135.h"

#define DHTTYPE DHT11     // DHT 22 (AM2302)

#define LAMP_PIN 32
#define LAMP_PIN_ON LOW
#define LAMP_PIN_OFF 1 - LAMP_PIN_ON

#define HUMIDIFIER_PIN 18
#define HUMIDIFIER_PIN_ON LOW 
#define HUMIDIFIER_PIN_OFF 1 - HUMIDIFIER_PIN_ON

#define FAN_PIN 27
#define FAN_PIN_ON HIGH // пропеллер нормально замкнутый, при отключении включается система СО2
#define FAN_PIN_OFF 1 - FAN_PIN_ON

#define CO2_PIN 35

#define HUMIDIFIER_TARGET_PIN 34
#define DHT_PIN  33
#define GROW_VEG_SWITCH_PIN 17

#define SCL_PIN 22
#define SDA_PIN 16

DHT dht(DHT_PIN, DHTTYPE); // temp and hum
RTC_DS1307 rtc; // real time clock
LiquidCrystal_I2C lcd(0x27, 16, 2); // display
MQ135 co2Sensor = MQ135(CO2_PIN);

int lampRelayState = 0;
int lampMode = 0; // 0 - veg, 1 - bloom, 2 - on, 3 - off

int humidifierState = 0;
int targetHumidity = 0;

int fanState = 1; // пропеллер нормально замкнутый, поэтому состояние и управление инверсное
