#ifndef growbox_display_h
#define growbox_display_h

#include "readings.h"
#include "options.h"
#include "devices.h"
#include <LiquidCrystal_I2C.h>

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
  lcd.print("Fan: ");
  lcd.print(r->fanState);
  lcd.printf(", СO2: ");
  lcd.print(r->co2);
  lcd.print(LCD_FILLER);
}

void displayRoundedFloat(float v)
{
  if (isnan(v))
    lcd.print(v);
  else
    lcd.print((int)v);

}

void displayTH(Readings *r)
{
  lcdFirstLine();  
  lcd.printf("T: ");
  displayRoundedFloat(r->temperature);
  lcd.print(LCD_FILLER);

  lcdSecondLine();  
  lcd.printf("H: ");
  displayRoundedFloat(r->humidity);
  lcd.print(" / ");
  displayRoundedFloat(r->targetHumidity);
  lcd.print(", R: ");
  lcd.print(r->humidifierState);
  lcd.print(LCD_FILLER);  
}

void displayLamp(Readings *r)
{
  lcdFirstLine();  
  lcd.print("M: ");
  switch (r->lampMode)
  {
    case 0: 
      lcd.print("Veg");
      break;
    case 1: 
      lcd.print("Bloom");
      break;
    case 2: 
      lcd.print("On");
      break;
    case 3: 
      lcd.print("Off");
      break;
    default:
      break;
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
    
     default:
      break;
  }

  displayMode++;
  displayMode %= 3;
}

#endif