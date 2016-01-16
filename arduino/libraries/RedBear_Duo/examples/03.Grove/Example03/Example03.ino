// -----------------------------------
// Example - 03: Controlling the RGB LED with Acceleration Sensor
// -----------------------------------
#include "application.h"
#include "MMA7660.h"
#include "ChainableLED.h"

MMA7660 accelemeter;
#define NUM_LEDS  1
ChainableLED leds(D4, D5, NUM_LEDS);

// This routine runs only once upon reset
void setup()
{
  accelemeter.init();                           // initialize the g-sensor
  leds.setColorRGB(0, 0, 0, 0);                 // turn down the LED
}

// This routine loops forever
void loop()
{
  int8_t x, y, z;

  accelemeter.getXYZ(&x,&y,&z);                 // read the g-sensor
  // convert acc data for LED display
  x = x - 32;
  x = abs(x) * 8;
  y = y - 32;
  y = abs(y) * 8;
  z = z - 32;
  z = abs(z) * 8;
  leds.setColorRGB(0, x, y, z);                 // write LED data
  delay(50);
}
