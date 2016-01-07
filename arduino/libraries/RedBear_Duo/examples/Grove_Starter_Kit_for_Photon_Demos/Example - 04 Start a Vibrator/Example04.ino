// -----------------------------------
// Example - 04: Start a Vibrator
// -----------------------------------
#include "application.h"
// name the pins
#define BUTTONPIN D2
#define MOTORPIN A4

// This routine runs only once upon reset
void setup()
{
  pinMode(BUTTONPIN, INPUT);                            
  pinMode(MOTORPIN, OUTPUT);                          
}

// This routine loops forever
void loop()
{
  int val = digitalRead(BUTTONPIN);               // read the hall sensor pin

  if(val == 0)                                  // if magnet detected
    digitalWrite(MOTORPIN, LOW);               // let the motor vibrate
  else
    digitalWrite(MOTORPIN, HIGH);                // stop it
  delay(50);
}
