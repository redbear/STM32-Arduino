// -----------------------------------
// Example - 05: Measuring Temperature
// -----------------------------------
#include "application.h"
#include <math.h>
#include "TM1637.h"

void dispNum(unsigned int num);

// name the pins
#define CLK D4
#define DIO D5
TM1637 tm1637(CLK,DIO);
#define TEMPPIN A4

// This routine runs only once upon reset
void setup()
{
  tm1637.set();                                 // cofig TM1637
  tm1637.init();                                // clear the display
  Serial.begin(9600);                           // init serial port on USB interface
}

// This routine loops forever
void loop()
{
  int B = 3975;                                 // B value of the thermistor

  int analogValue = analogRead(TEMPPIN);        // read rotary pin
  float resistance=(float)(4095-analogValue)*10000/analogValue;   //get the resistance of the sensor
  float temperature=1/(log(resistance/10000)/B+1/298.15)-273.15;  //convert to temperature via datasheet
  Serial.print("analogValue: ");
  Serial.println(analogValue);
  Serial.print("resistance: ");
  Serial.println(resistance);
  Serial.print("temperature: ");
  Serial.println(temperature);
  Serial.println("");
  dispNum((unsigned int) (temperature + 0.5));  // display the voltage
  delay(200);
}

// display a integer value less then 10000
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
