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
 
#ifndef PIN_INF_H_
#define PIN_INF_H_

#if defined(ARDUINO) 
#include "Arduino.h"
#endif

#define TOTAL_PINS_NUM    18
#define VERSION_BLINK_PIN 7

#define IS_PIN_DIGITAL(p) ( (p) >= 0 && (p) < 18 )
#define IS_PIN_ANALOG(p)  ( (p) >= 8 && (p) < 16 )
#define IS_PIN_PWM(p)     ( ( (p) >= 0 && (p) < 5 ) || (p) == 8 || (p) == 9 || ( (p) >= 14 && (p) < 18 ) )
#define IS_PIN_SERVO(p)   ( (p) == 12 || (p) == 13 )

#define PIN_TO_DIGITAL(p) (p)
#define PIN_TO_ANALOG(p)  (p)
#define PIN_TO_PWM(p)     (p)
#define PIN_TO_SERVO(p)   (p)

#endif

