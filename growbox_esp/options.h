#ifndef growbox_options_h
#define growbox_options_h

// #define RTC_UPDATE_TIME // использовать один раз для инициализации модуля времени. В боевом режиме закомментарить обязательно!!!

#define LOG_TIME
#define LOG_DHT
#define LOG_LAMP_MODE
#define LOG_HTTP

#define DEVICE_LOCATION "Dungeon"

#define DHTTYPE DHT22     // DHT 22 (AM2302)

#define LAMP_PIN A7
#define HUMIDIFIER_PIN A4
#define DHT_PIN A5
#define GROW_VEG_SWITCH_PIN A17
#define SCL_PIN A15
#define SDA_PIN A16

int lampNightStartHour[2] = {12, 11};
int lampDayStartHour[2] = {18, 23};

#define READINGS_ARCHIVE_LENGTH 100

#define WIFI_SSID  "Trifon"
#define WIFI_PASSWORD  "18000dong"
#define WIFI_DIAGNOSTIC_DATA_URL "http://192.168.1.9:9000" // Kibbutz
#define WIFI_POST_DATA_URL "http://192.168.2.100:9000" // Melchior

#define ARCHIVE_TIME_SECONDS 180

#define MIN_HUMIDITY 45
#define MAX_HUMIDITY 55

#endif