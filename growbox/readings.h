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
    float soilHumidityRaw;
    int lampMode; // 0 - veg, 1 - bloom, 2 - on, 3 - off
    bool lampRelayState;
    bool wateringState;
    uint32_t wateringLastTime;
    bool humidifierState;

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
    soilHumidityRaw = NAN;
    lampMode = 0;
    lampRelayState = false;
    wateringState = false;
    humidifierState = false;
    wateringLastTime = 0;
  }

  void assign(Readings *source)
  {
    dt = source->dt;
    temperature = source->temperature;
    humidity = source->humidity;
    soilHumidity = source->soilHumidity;    
    soilHumidityRaw = source->soilHumidityRaw;    
    lampMode = source->lampMode;
    lampRelayState = source->lampRelayState;
    humidifierState = source->humidifierState;
    wateringState = source->wateringState;
    wateringLastTime = source->wateringLastTime;
  }

  void setSoilHumidityRaw(float shr)
  {
    soilHumidityRaw = shr;             // Читаем сырые данные с датчика,
    soilHumidity = map(soilHumidityRaw, soilHumiditiSensorMin, soilHumiditiSensorMax, 0, 100);  // адаптируем значения от 0 до 100
    if (soilHumidity > 100 && soilHumidity < 120)
      soilHumidity = 100;

    if (soilHumidity < 0 && soilHumidity > -20)
      soilHumidity = 0;    
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
    DateTime dtWateringLastTime(wateringLastTime);

    String res = "{\r\n" +
      formatForJSON("dtStr", forStr.timestamp()) +
      formatForJSON("dt", String(dt)) + 
      formatForJSON("lampMode", String((int)lampMode)) + 
      formatForJSON("lampState", String((int)lampRelayState)) +
      formatForJSON("humidifierState", String((int)humidifierState)) +      
      formatForJSON("wateringState", String((int)wateringState)) +
      formatForJSON("wateringLastTime", dtWateringLastTime.timestamp());

    if (!isnan(temperature))
      res += formatForJSON("temperature", floatToStr(temperature));

    if (!isnan(humidity))
      res += formatForJSON("humidity", floatToStr(humidity));

    if (!isnan(soilHumidity) && soilHumidity >= 0 && soilHumidity <= 100)
      res += formatForJSON("soilHumidity", floatToStr(soilHumidity));

    if (!isnan(soilHumidityRaw))
      res += formatForJSON("soilHumidityRaw", floatToStr(soilHumidityRaw));

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