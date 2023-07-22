#include <LCD_I2C.h>
#include "DHT.h"
#include "Wire.h"

#define PIN_RELAY 2
#define PIN_DHT 3

#define MIN_COMPRESSOR_SECONDS_OFF 300
#define MIN_COMPRESSOR_SECONDS_ON 60

#define T_LOW 8
#define T_HIGH 10

#define STRAIGHT_RELAY false
#define CYCLE_SECONDS 3

DHT dht(PIN_DHT, DHT11);
LCD_I2C lcd(0x27);
bool relayState, canRelayOn, canRelayOff;
unsigned long cyclesCounter;
byte displayInfo;

void doSetRelay(bool state)
{
  relayState = state;
  canRelayOn = false;
  canRelayOff = false;
  cyclesCounter = 0;

  int signal = STRAIGHT_RELAY ? (state ? HIGH : LOW) : (state ? LOW : HIGH);  
  digitalWrite(PIN_RELAY, signal);
}

void setRelay(bool state)
{
  if (relayState == state) 
    return;

  if ((state && canRelayOn) || (!state && canRelayOff))
    doSetRelay(state);
}

void setup() 
{  
  displayInfo = 0;
  pinMode(PIN_RELAY, OUTPUT); // Объявляем пин реле как выход  
  doSetRelay(false);
  
  dht.begin();
  
  lcd.begin();                      // Инициализация дисплея  
  lcd.backlight();                 // Подключение подсветки
}

void printTemperature(float t)
{
  lcd.setCursor(0, 0);
  lcd.print("T: ");
  if (isnan(t))
    lcd.print("--");
  else
    lcd.print((int)t);

  lcd.print(" / (");
  lcd.print(T_LOW);
  lcd.print("..");
  lcd.print(T_HIGH);
  lcd.print(")    ");
}

void checkRelay(float t)
{
  if (isnan(t) || t <= T_LOW)
    setRelay(false);
  else
  if (t >= T_HIGH)
    setRelay(true); 
}

void printHumidity(float h)
{
  lcd.setCursor(0, 1);
  lcd.print("Hg: ");
  if (isnan(h))
    lcd.print("--");
  else
  {
    lcd.print((int)h);
    lcd.print("%");
  }

  lcd.print("          ");
  
  lcd.setCursor(14, 1);
  if (relayState)
    lcd.print("R1");
  else
    lcd.print("R0"); 
}

void checkControlTimes()
{
  if (cyclesCounter >= MIN_COMPRESSOR_SECONDS_ON)
    canRelayOff = true;

  if (cyclesCounter >= MIN_COMPRESSOR_SECONDS_OFF)
    canRelayOn = true;

   if (canRelayOn && canRelayOff)
     cyclesCounter = 0;
   else
     cyclesCounter += CYCLE_SECONDS;
}

void printStats()
{
  lcd.setCursor(0, 0);
  lcd.print("Cycles: ");
  lcd.print(cyclesCounter);
  lcd.print("     ");
  if (cyclesCounter < 100)
    lcd.print(" ");
  if (cyclesCounter < 10)
    lcd.print(" ");

  lcd.setCursor(0, 1);
  lcd.print("?On: ");
  lcd.print((int)canRelayOn);
  lcd.print(", ?Off: ");
  lcd.print((int)canRelayOff);
}

void printTimes()
{ 
  lcd.setCursor(0, 0); 
  lcd.print("Time on: "); 
  lcd.print(MIN_COMPRESSOR_SECONDS_ON);
  lcd.print("      ");
  lcd.setCursor(0, 1);
  lcd.print("Time off: "); 
  lcd.print(MIN_COMPRESSOR_SECONDS_OFF);
  lcd.print("      ");
}

void printInfo(float t, float h)
{
  switch (displayInfo)
  {
    case 0:
      printTemperature(t);  
      printHumidity(h);
      break;
      
     case 1:
       printStats();
       break;
     case 2:  
       printTimes();
       break;
  }
  
  displayInfo++;
  displayInfo %= 2;
}

void loop() {
  checkControlTimes();
  float t = dht.readTemperature();
  checkRelay(t);
  
  float h = dht.readHumidity();
  printInfo(t, h);
   
  delay(CYCLE_SECONDS * 1000);
}
