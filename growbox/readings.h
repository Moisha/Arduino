#ifndef growbox_readings_h
#define growbox_readings_h

#include "options.h"
#include "utils.h"

class Readings;

Readings *readingsArchive[READINGS_ARCHIVE_LENGTH];

class Readings
{
  public:
    uint32_t dt;
    float temperature;
    float humidity;
    float soilHumidity;
    int lampMode; // 0 - grow, 1 - veg
    bool lampRelayState;
    bool wateringState;

  Readings() 
  { 
      init();
  };

  void init()
  {
    dt = 0;
    temperature = NAN;
    humidity = NAN;
    soilHumidity = NAN;
    lampMode = 0;
    lampRelayState = false;
    wateringState = false;
  }

  void assign(Readings *source)
  {
    dt = source->dt;
    temperature = source->temperature;
    humidity = source->humidity;
    soilHumidity = source->soilHumidity;    
    lampMode = source->lampMode;
    lampRelayState = source->lampRelayState;
    wateringState = source->wateringState;
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
      formatForJSON("dtStr", forStr.timestamp()) +
      formatForJSON("dt", String(dt)) + 
      formatForJSON("lampMode", String((int)lampMode)) + 
      formatForJSON("lampState", String((int)lampRelayState)) +
      formatForJSON("wateringState", String((int)wateringState));

    if (!isnan(temperature))
      res += formatForJSON("temperature", floatToStr(temperature));

    if (!isnan(humidity))
      res += formatForJSON("humidity", floatToStr(humidity));

    if (!isnan(soilHumidity) && soilHumidity >= 0 && soilHumidity <= 100)
      res += formatForJSON("soilHumidity", floatToStr(soilHumidity));

    res = res.substring(0, res.length() - 2) + "\n"; // уберем запятую в конце

    res += "}";

    return res;
  }
};

Readings* prepareReadings()
{
  for (int i = READINGS_ARCHIVE_LENGTH - 1; i >= 1 ; i--)  
    readingsArchive[i]->assign(readingsArchive[i - 1]);

  Readings *r = readingsArchive[0];  
  r->init();

  return r;
}

#endif