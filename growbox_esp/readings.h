#ifndef growbox_readings_h
#define growbox_readings_h

#include "options.h"
#include "utils.h"
#include <RTClib.h>

class Readings;

Readings *readingsArchive[READINGS_ARCHIVE_LENGTH];

class Readings
{
  public:
    uint32_t dt;
    float temperature;
    float humidity;
    float targetHumidity;
    float co2;
    int lampMode; // 0 - veg, 1 - bloom, 2 - on, 3 - off
    bool lampRelayState;
    bool humidifierState;
    bool fanState;

  Readings() 
  { 
      init();
  };

  void init()
  {
    dt = 0;
    temperature = NAN;
    humidity = NAN;
    targetHumidity = NAN;
    co2 = NAN;
    lampMode = 0;
    lampRelayState = false;
    humidifierState = false;
    fanState = true;
  }

  void assign(Readings *source)
  {
    dt = source->dt;
    temperature = source->temperature;
    humidity = source->humidity;
    targetHumidity = source->targetHumidity;
    co2 = source->co2;
    lampMode = source->lampMode;
    lampRelayState = source->lampRelayState;
    humidifierState = source->humidifierState;
    fanState = source->fanState;
  }

  String formatForJSON(String name, String val)
  {
    return "\"" + name + "\": \"" + val + "\",\n";
  }

  String toJSON()
  {
    if (dt <= 0)
      return "";

    DateTime forStr(dt);

    String res = "{\r\n" +
      formatForJSON("source", DEVICE_LOCATION) +
      formatForJSON("dtStr", forStr.timestamp()) +
      formatForJSON("dt", String(dt)) + 
      formatForJSON("lampMode", String((int)lampMode)) + 
      formatForJSON("lampState", String((int)lampRelayState)) +
      formatForJSON("humidifierState", String((int)humidifierState)) +
      formatForJSON("fanState", String((int)fanState));

    if (!isnan(temperature))
      res += formatForJSON("temperature", floatToStr(temperature));

    if (!isnan(humidity))
      res += formatForJSON("humidity", floatToStr(humidity));

    if (!isnan(targetHumidity))
      res += formatForJSON("targetHumidity", floatToStr(targetHumidity));

    if (!isnan(co2))
      res += formatForJSON("co2", floatToStr(co2));

    res = res.substring(0, res.length() - 2) + "\n"; // уберем запятую в конце

    res += "}";

    return res;
  }
};

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

#endif