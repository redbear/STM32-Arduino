// -----------------------------------
// Project - 01 Direction Recorder
// -----------------------------------
#include "application.h"
#include <math.h>
#include "MMA7660.h"
#include "ChainableLED.h"

// name the pins
#define BUTTONPIN D2
#define MOTORPIN A4
#define LIGHTPIN A0
MMA7660 accelemeter;
#define NUM_LEDS  1
ChainableLED leds(D4, D5, NUM_LEDS);

int8_t x_r, y_r, z_r;                           // original location by user set

// This routine runs only once upon reset
void setup()
{
  Serial.begin(9600);                           // init serial port on USB interface
  pinMode(BUTTONPIN, INPUT);                    // set user key pin as input
  pinMode(MOTORPIN, OUTPUT);
  accelemeter.init();                           // initialize the g-sensor
  leds.setColorRGB(0, 0, 0, 0);                 // turn down the LED
}

// This routine loops forever
void loop()
{
  int8_t x, y, z;
  float ax,ay,az;

  //read each input pins and sensors
  int keyValue = digitalRead(BUTTONPIN);        // read the hall sensor pin
  int lightValue = analogRead(LIGHTPIN);        // read light sensor pin
  accelemeter.getXYZ(&x,&y,&z);                 // read the g-sensor
  Serial.print(x);
  Serial.print('\t');
  Serial.print(y);
  Serial.print('\t');
  Serial.print(z);
  Serial.println("");

  accelemeter.getAcceleration(&ax,&ay,&az);
  Serial.println("accleration of X/Y/Z: ");
  Serial.print(ax);
  Serial.println(" g");
  Serial.print(ay);
  Serial.println(" g");
  Serial.print(az);
  Serial.println(" g");
  Serial.println("*************");

  // set the original location
  if(keyValue == 1)               // if the button was pressed
  {
    x_r = x;
    y_r = y;
    z_r = z;
  }

  // if the environment light intensity reaches a certain value
  if(lightValue > 1000)
  {
    // if the accelerometer is back to near the original location
    if((abs(x_r - x) < 5) && (abs(x_r - x) < 5) && (abs(x_r - x) < 5))
    {
      digitalWrite(MOTORPIN, HIGH);             // let the motor vibrate
    }
    else
    {
      digitalWrite(MOTORPIN, LOW);              // stop it
    }
  }
  else
  {
    digitalWrite(MOTORPIN, LOW);                // stop it
  }

  // convert acc data for LED display
  x = x - 32;
  x = abs(x) * 8;
  y = y - 32;
  y = abs(y) * 8;
  z = z - 32;
  z = abs(z) * 8;
  leds.setColorRGB(0, x, y, z);                 // write LED data

  delay(100);
}
