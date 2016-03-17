// -----------------------------------
// Project - 02 Temperature Alarm
// -----------------------------------

#include <math.h>
#include "TM1637.h"

#if defined(ARDUINO) 
SYSTEM_MODE(MANUAL);//do not connect to cloud
#else
SYSTEM_MODE(AUTOMATIC);//connect to cloud
#endif

void dispNum(int num);

// name the pins
#define CLK D4
#define DIO D5
TM1637 tm1637(CLK,DIO);
#define TEMPPIN A4
#define ROTARYPIN A0
#define BUZZERPIN D1

int tempMesured;                                // the temperature we mesured
int tempUserSet;                                // the temperature user set
int tempThreshould;                             // the temperature threshould
int timeout = 0;                                // for display delay

// This routine runs only once upon reset
void setup()
{
  Serial.begin(9600);                           // init serial port on USB interface
  tm1637.set();                                 // cofig TM1637
  tm1637.init();                                // clear the display
  pinMode(BUZZERPIN, OUTPUT);                   // set buzzer pin as output
}

// This routine loops forever
void loop()
{
  int B = 3975;                                 // B value of the thermistor

  int tempValue = analogRead(TEMPPIN);          // read temperature adc
  float resistance=(float)(4095-tempValue)*10000/tempValue;       // get the resistance of the sensor
  float temperature=1/(log(resistance/10000)/B+1/298.15)-273.15;  // convert to temperature via datasheet
  Serial.print("analogValue: ");
  Serial.println(tempValue);
  Serial.print("resistance: ");
  Serial.println(resistance);
  Serial.print("temperature: ");
  Serial.println(temperature);
  Serial.println("");
  tempMesured = (int) (temperature + 0.5);

  int rotaryValue = analogRead(ROTARYPIN);  // read rotary pin
  tempUserSet = (4095 - rotaryValue) * (100 + 20) / 4096 - 20;
  Serial.print("temperature threshoul user set: ");
  Serial.println(tempUserSet);

  if(tempThreshould != tempUserSet)             // if user rotates the rotary and the tempUserSet changed
  {
    tempThreshould = tempUserSet;               // update threshould with user set
    timeout = 2000;                             // delay 2000ms to display tempUserSet
  }

  if(timeout > 0)                               // select which temperature to display
  {
    dispNum(tempUserSet);
  }
  else
  {
    dispNum(tempMesured);
  }

  // if only temperature is higher and magnet is detected
  if(tempMesured >= tempUserSet)
  {
    digitalWrite(BUZZERPIN, HIGH);              // make a alert
  }
  else
  {
    digitalWrite(BUZZERPIN, LOW);               // stop the alert
  }


  delay(200);
  timeout -= 200;
  if(timeout < 0) timeout = 0;
}

// display a integer value less then 10000
void dispNum(int num)
{
  int8_t TimeDisp[] = {0, 0, 0, 0};             // limit the maximum number
  int numabs;

  if(num > 9999) num = 9999;
  if(num < -999) num = -999;
  numabs = abs(num);

  TimeDisp[0] = numabs / 1000;
  TimeDisp[1] = numabs % 1000 / 100;
  TimeDisp[2] = numabs % 100 / 10;
  TimeDisp[3] = numabs % 10;

  if(num < 0)                                   // if a negative to be display
    TimeDisp[0] = 10;                           // show 'A' at the first digit

  tm1637.display(TimeDisp);
}
