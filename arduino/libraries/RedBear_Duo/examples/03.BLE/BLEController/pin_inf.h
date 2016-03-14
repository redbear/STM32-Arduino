
#ifndef PIN_INF_H_
#define PIN_INF_H_

#include "Arduino.h"

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

