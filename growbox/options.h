#ifndef growbox_options_h
#define growbox_options_h

// #define RTC_UPDATE_TIME // использовать один раз для инициализации модуля времени. В боевом режиме закомментарить обязательно!!!

#define LOG_TIME
#define LOG_DHT
#define LOG_WATER
#define LOG_LAMP_MODE
#define LOG_HTTP

int lampNightStartHour[2] = {22, 20};
int lampDayStartHour[2] = {2, 8};

#define READINGS_ARCHIVE_LENGTH 100

#define WIFI_SSID  "Trifon"
#define WIFI_PASSWORD  "18000dong"
#define WIFI_DIAGNOSTIC_DATA_URL "http://192.168.1.9:9000" // Kibbutz
#define WIFI_POST_DATA_URL "http://192.168.1.10:9000" // Melchior

#define ARCHIVE_TIME_SECONDS 180

const float dailyWateringVolume = 100; // тип обязательно float, чтобы не попасть на целочисленное деление
const float wateringPerMinute = 125;

int soilHumiditiSensorMin = 970;                    // Определяем минимальное показание датчика (в воздухе),
int soilHumiditiSensorMax = 680;     

#endif