// -----------------------------------
// Controlling the RGB LED 
// -----------------------------------
#include "application.h"
#include "ChainableLED.h"

#if defined(ARDUINO) 
SYSTEM_MODE(MANUAL);//do not connect to cloud
#else
SYSTEM_MODE(AUTOMATIC);//connect to cloud
#endif

#define NUM_LEDS  1
ChainableLED rgb(D4, D5, NUM_LEDS);

// This routine runs only once upon reset
void setup()
{
  rgb.setColorRGB(0, 0, 0, 0);                 // turn down the LED
}

// This routine loops forever
void loop()
{
    rgb.setColorRGB(0,50, 0, 0);                 // turn down the LED
    delay(500);
    rgb.setColorRGB(0, 0 ,50,  0);                 // turn down the LED
    delay(500);
    rgb.setColorRGB(0, 0 , 0, 50);                 // turn down the LED
    delay(500);
}

