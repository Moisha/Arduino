#ifndef growbox_display_h
#define growbox_display_h

#include "readings.h"
#include "options.h"

LiquidCrystal_I2C lcd(0x27, 16, 2); // display
int displayMode = 0;
#define LCD_FILLER "                "

void lcdFirstLine()
{
  lcd.setCursor(0, 0);    
}

void lcdSecondLine()
{
  lcd.setCursor(0, 1);    
}

void displayDt(Readings *r)
{
  DateTime dt(r->dt);

  lcdFirstLine();  
  lcd.print(dt.timestamp(DateTime::TIMESTAMP_TIME));    
  lcd.print(LCD_FILLER);

  lcdSecondLine();  
  lcd.printf("Mem: ");
  lcd.print(ESP.getFreeHeap());
  lcd.print(LCD_FILLER);
}

void displayTH(Readings *r)
{
  lcdFirstLine();  
  lcd.printf("T: ");
  lcd.print(r->temperature);    
  lcd.print(LCD_FILLER);

  lcdSecondLine();  
  lcd.printf("H: ");
  lcd.print(r->humidity);
  lcd.print(LCD_FILLER);  
}

void displaySoil(Readings *r)
{
  lcdFirstLine();  
  lcd.print("Soil H: ");
  if (isnan(r->soilHumidity))
    lcd.print("---");
  else
    lcd.print(r->soilHumidity);

  lcd.print(LCD_FILLER);

  DateTime lastWatering(r->wateringLastTime);
  lcdSecondLine();  
  lcd.print("W: ");
  lcd.printf("%02d.%02d %02d:%02d", lastWatering.day(), lastWatering.month(), lastWatering.hour(), lastWatering.minute());
  lcd.print(LCD_FILLER);
}

void displayLamp(Readings *r)
{
  lcdFirstLine();  
  lcd.print("M: ");
  switch (r->lampMode)
  {
    case 0: lcd.print("Veg");
    case 1: lcd.print("Bloom");
    case 2: lcd.print("On");
    case 3: lcd.print("Off");
  }

  lcd.print(", R: ");
  lcd.print(r->lampRelayState);
  lcd.print(LCD_FILLER);

  lcdSecondLine();  
  lcd.print("D: ");
  lcd.print(lampDayStartHour[r->lampMode]);
  lcd.print(", N: ");
  lcd.print(lampNightStartHour[r->lampMode]);
  lcd.print(LCD_FILLER);  
}

void initDisplay()
{
  Wire.begin(D2, D1);
  lcd.begin(D2, D1);

  lcd.clear();         
  lcd.backlight();      // Make sure backlight is on  

  lcd.setCursor(0, 0);  
}

void displayValues(Readings *r)
{
  switch (displayMode)
  {
    case 0:
      displayDt(r);
      break;

    case 1:
      displayTH(r);
      break;

    case 2:
      displayLamp(r);
      break;
    
    case 3:
      displaySoil(r);
      break;

    default:
      break;
  }

  displayMode++;
  displayMode %= 4;
}

#endif