#ifndef growbox_readings_h
#define growbox_readings_h

#include "options.h"

class Readings;

Readings *readingsArchive[READINGS_ARCHIVE_LENGTH];

class Readings
{
  public:
    uint32_t dt;
    float temperature;
    float humidity;
    float soilHumidity;

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
  }

  void assign(Readings *source)
  {
    dt = source->dt;
    temperature = source->temperature;
    humidity = source->humidity;
    soilHumidity = source->soilHumidity;    
  }

  String toJSON(int lastTime)
  {
    float sumT = 0;
    float sumH = 0;
    float sumSH = 0;    

    float countT = 0;
    float countH = 0;
    float countSH = 0;    

    for (int i = 0; i < READINGS_ARCHIVE_LENGTH; i++)
    {
      Readings *r = readingsArchive[i];
      if (r->dt <= lastTime || r->dt == 0)
        break;

      if (!isnan(r->temperature))
      {
        sumT += r->temperature;
        countT ++;
      }
      
      if (!isnan(r->humidity))
      {
        sumH += r->humidity;
        countH ++;
      }

      if (!isnan(r->soilHumidity))
      {
        sumSH += r->soilHumidity;
        countSH ++;
      }
    }

    String res = "{";
    return res;
  }
};

#endif