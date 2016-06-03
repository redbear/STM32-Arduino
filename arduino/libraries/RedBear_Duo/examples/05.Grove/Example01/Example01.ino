/*
 * Copyright (c) 2016 RedBear
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), 
 * to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS 
 * IN THE SOFTWARE.
 */

/*
 * Example - 01: Sing a song using buzzer
 */
 
#include "application.h"

/*
 * SYSTEM_MODE:
 *     - AUTOMATIC: Automatically try to connect to Wi-Fi and the Particle Cloud and handle the cloud messages.
 *     - SEMI_AUTOMATIC: Manually connect to Wi-Fi and the Particle Cloud, but automatically handle the cloud messages.
 *     - MANUAL: Manually connect to Wi-Fi and the Particle Cloud and handle the cloud messages.
 *     
 * SYSTEM_MODE(AUTOMATIC) does not need to be called, because it is the default state. 
 * However the user can invoke this method to make the mode explicit.
 * Learn more about system modes: https://docs.particle.io/reference/firmware/photon/#system-modes .
 */
#if defined(ARDUINO) 
SYSTEM_MODE(SEMI_AUTOMATIC); 
#endif

// name the pins
#define BUTTONPIN D2
#define BUZZERPIN D1

int melody[] = { 1908, 2551, 2551, 2273, 2551, 0, 2024, 1908};  // notes in the melody
int noteDurations[] = { 4, 8, 8, 4, 4, 4, 4, 4 };               // note durations

// This routine runs only once upon reset
void setup() {
  pinMode(BUTTONPIN, INPUT);                            // set user key pin as input
}

// This routine loops forever
void loop() {
  if (digitalRead(BUTTONPIN) == 1) {                     // if the button was pressed
    for (int thisNote = 0; thisNote < 8; thisNote++) {   // ergodic all notes
      int noteDuration = 1000 / noteDurations[thisNote]; // calculate the note duration
      tone(BUZZERPIN, melody[thisNote], noteDuration);   // let speaker sonds
      int pauseBetweenNotes = noteDuration * 1.30;       // set a minimum time between notes
      delay(pauseBetweenNotes);                          // delay for the while
      noTone(BUZZERPIN);                                 // stop the tone playing:
    }
  }
}
