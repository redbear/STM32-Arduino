// -----------------------------------
// Example - 06: It’s Time to Get Up
// -----------------------------------
#include "application.h"

// name the pins
#define LIGHTPIN A0
#define BUZZERPIN D1

// This routine runs only once upon reset
void setup()
{
  pinMode(BUZZERPIN, OUTPUT);                   // set user key pin as input
  Serial.begin(9600);
}

// This routine loops forever
void loop()
{
  int analogValue = analogRead(LIGHTPIN);       // read light sensor pin
  Serial.print("light strength: ");
  Serial.println(analogValue);
  if(analogValue > 1000)                        // if it is bright enough
    digitalWrite(BUZZERPIN, HIGH);              // let the buzzer chirp
  else
    digitalWrite(BUZZERPIN, LOW);               // stop it
  delay(500);
}
