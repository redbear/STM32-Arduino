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
 * Example - 02: Display the read analog value on TM1637
 */
 
#include "application.h"
#include "TM1637.h"

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
#define ROTARYPIN A0
#define CLK D4
#define DIO D5

void dispNum(unsigned int num);

TM1637 tm1637(CLK,DIO);

// This routine runs only once upon reset
void setup() {
  tm1637.set();                                 // cofig TM1637
  tm1637.init();                                // clear the display
}

// This routine loops forever
void loop() {
  int analogValue = analogRead(ROTARYPIN);        // read rotary pin
  int voltage = (long)3300 *  analogValue / 4096; // calculate the voltage
  dispNum(voltage);                               // display the voltage
  delay(200); //this delay on user manual is 50ms, sometimes it may result in flicker on Grove - 4-Digit Display. long delay time will reduce the reduce this appearance.
}

//display a integer value less then 10000
void dispNum(unsigned int num) {
  int8_t TimeDisp[] = {0x01,0x02,0x03,0x04};    // limit the maximum number

  if (num > 9999) num = 9999;
  TimeDisp[0] = num / 1000;
  TimeDisp[1] = num % 1000 / 100;
  TimeDisp[2] = num % 100 / 10;
  TimeDisp[3] = num % 10;
  tm1637.display(TimeDisp);
}
