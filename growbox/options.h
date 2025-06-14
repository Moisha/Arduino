#ifndef growbox_options_h
#define growbox_options_h

// #define RTC_UPDATE_TIME // использовать один раз для инициализации модуля времени. В боевом режиме закомментарить обязательно!!!

#define LOG_TIME
#define LOG_DHT
// #define LOG_WATER
#define LOG_LAMP_MODE
#define LOG_HTTP

int lampNightStartHour[2] = {12, 10};
int lampDayStartHour[2] = {18, 22};

#define READINGS_ARCHIVE_LENGTH 100

#define DEVICE_LOCATION "Fridge"

#define WIFI_SSID  "TrifonG"
#define WIFI_PASSWORD  "18000dong"
// #define WIFI_DIAGNOSTIC_DATA_URL "http://192.168.1.9:9000" // Kibbutz
#define WIFI_POST_DATA_URL "http://192.168.1.100:9000" // Melchior

#define ARCHIVE_TIME_SECONDS 180

#define MIN_HUMIDITY 45
#define MAX_HUMIDITY 55

const float dailyWateringVolume = 2500.0; // тип обязательно float, чтобы не попасть на целочисленное деление
const int daysBetweenWaterings = 2;
const float wateringPerMinute = 55.0;

const int soilHumiditiSensorMin = 970;                    // Определяем минимальное показание датчика (в воздухе),
const int soilHumiditiSensorMax = 680;     

#endif