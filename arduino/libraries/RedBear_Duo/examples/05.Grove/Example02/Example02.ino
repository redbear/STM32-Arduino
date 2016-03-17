// -----------------------------------
// Example - 02: Display the Analog Value
// -----------------------------------
#include "application.h"
#include "TM1637.h"

#if defined(ARDUINO) 
SYSTEM_MODE(MANUAL);//do not connect to cloud
#else
SYSTEM_MODE(AUTOMATIC);//connect to cloud
#endif


// name the pins
#define ROTARYPIN A0
#define CLK D4
#define DIO D5

void dispNum(unsigned int num);

TM1637 tm1637(CLK,DIO);

// This routine runs only once upon reset
void setup()
{
  tm1637.set();                                 // cofig TM1637
  tm1637.init();                                // clear the display
}

// This routine loops forever
void loop()
{
  int analogValue = analogRead(ROTARYPIN);      // read rotary pin
  int voltage = (long)3300 *  analogValue / 4096;     // calculate the voltage
  dispNum(voltage);                             // display the voltage
  delay(200);//this delay on user manual is 50ms, sometimes it may result in flicker on Grove - 4-Digit Display. long delay time will reduce the reduce this appearance.
}

//display a integer value less then 10000
void dispNum(unsigned int num)
{
  int8_t TimeDisp[] = {0x01,0x02,0x03,0x04};    // limit the maximum number

  if(num > 9999) num = 9999;
  TimeDisp[0] = num / 1000;
  TimeDisp[1] = num % 1000 / 100;
  TimeDisp[2] = num % 100 / 10;
  TimeDisp[3] = num % 10;
  tm1637.display(TimeDisp);
}
