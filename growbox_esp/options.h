#ifndef growbox_options_h
#define growbox_options_h

// #define RTC_UPDATE_TIME // использовать один раз для инициализации модуля времени. В боевом режиме закомментарить обязательно!!!

#define LOG_TIME
#define LOG_DHT
#define LOG_LAMP_MODE
#define LOG_HTTP
#define LOG_COMMON

#define DEVICE_LOCATION "Dungeon"

int lampNightStartHour[2] = {12, 11};
int lampDayStartHour[2] = {18, 23};

#define READINGS_ARCHIVE_LENGTH 100

#define WIFI_SSID  "TrifonK"
#define WIFI_PASSWORD  "18000dong"
// #define WIFI_DIAGNOSTIC_DATA_URL "http://192.168.1.9:9000" // Kibbutz
#define WIFI_POST_DATA_URL "http://192.168.2.100:9000" // Melchior

#define ARCHIVE_TIME_SECONDS 180

#define DEFAULT_HUMIDITY 50
#define HUMIDITY_HISTERESIS 5

#define HUMIDIFIER_TARGET_MIN 40
#define HUMIDIFIER_TARGET_MAX 80

#endif