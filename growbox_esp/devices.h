#pragma once

#include <DHT.h>
#include <RTClib.h>
#include "options.h"

DHT dht(DHT_PIN, DHTTYPE); // temp and hum
RTC_DS1307 rtc; // real time clock

int lampRelayState = 0;
int lampMode = 0; // 0 - veg, 1 - bloom, 2 - on, 3 - off
int humidifierState = 0;
