#ifndef growbox_options_h
#define growbox_options_h

#define RTC_CONNECTED 
// #define RTC_UPDATE_TIME 

#define LOG_TIME
#define LOG_DHT
#define LOG_WATER
#define LOG_LAMP_MODE

int lampNightStartHour[2] = {22, 20};
int lampDayStartHour[2] = {3, 8};

#define READINGS_ARCHIVE_LENGTH 100

#define WIFI_SSID  "Trifon"
#define WIFI_PASSWORD  "18000dong"
#define WIFI_POST_DATA_URL "http://192.168.1.10:9000/postreadings"

#define ARCHIVE_TIME_SECONDS 180

const float dailyWateringVolume = 100;
const float wateringPerMinute = 125;

#endif