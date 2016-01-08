// -----------------------------------
// Example - 01: Singing A Song
// -----------------------------------
#include "application.h"

// name the pins
#define BUTTONPIN D2
#define BUZZERPIN D1

int melody[] = {1908,2551,2551,2273,2551,0,2024,1908};  // notes in the melody
int noteDurations[] = {4,8,8,4,4,4,4,4 };               // note durations

// This routine runs only once upon reset
void setup()
{
  pinMode(BUTTONPIN, INPUT);                            // set user key pin as input
}

// This routine loops forever
void loop()
{
  if(digitalRead(BUTTONPIN) == 1) {                     // if the button was pressed
    for (int thisNote = 0; thisNote < 8; thisNote++) {  // ergodic all notes
      int noteDuration = 1000/noteDurations[thisNote];  // calculate the note duration
      tone(BUZZERPIN, melody[thisNote], noteDuration);  // let speaker sonds
      int pauseBetweenNotes = noteDuration * 1.30;      // set a minimum time between notes
      delay(pauseBetweenNotes);                         // delay for the while
      noTone(BUZZERPIN);                                // stop the tone playing:
    }
  }
}
