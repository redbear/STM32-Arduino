// -----------------------------------
// Controlling the RGB LED 
// -----------------------------------
#if defined(ARDUINO) 
SYSTEM_MODE(MANUAL);//do not connect to cloud
#else
SYSTEM_MODE(AUTOMATIC);//connect to cloud
#endif


// This routine runs only once upon reset
void setup()
{
  RGB.control(true);
}

// This routine loops forever
void loop()
{
    RGB.color(255, 0, 0);                 // turn down the LED
    delay(500);
    RGB.color(0, 255 ,0);                 // turn down the LED
    delay(500);
    RGB.color(0, 0,255);                 // turn down the LED
    delay(500);
}

