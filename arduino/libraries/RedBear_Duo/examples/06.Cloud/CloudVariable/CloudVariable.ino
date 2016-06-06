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
 * As long as your Duo is connecting to the Particle Cloud, it can expose a variable through the Particle Cloud 
 * so that you can get its value wherever you can access the internet.
 * 
 * Get the variable value from Particle Cloud using curl:
 *     curl "https://api.particle.io/v1/devices/{YOUR_DEVICE_ID}/{VARIABLE_NAME}?access_token={YOUR_ACCESS_TOKEN}" -k
 * E.g.(Assume that your Duo's device ID is: 112233445566778899aabbcc and your access token is 12345678901234567890):
 *     1. curl "https://api.particle.io/v1/devices/112233445566778899aabbcc/analog?access_token=12345678901234567890" -k
 *     2. curl "https://api.particle.io/v1/devices/112233445566778899aabbcc/counter?access_token=12345678901234567890" -k
 *     3. curl "https://api.particle.io/v1/devices/112233445566778899aabbcc/greet?access_token=12345678901234567890" -k
 *     
 * Your access token can be found on Particle Build: https://build.particle.io, After you login in, check it under the "Settings" tag.
 * Learn more about cloud variable: https://docs.particle.io/reference/firmware/photon/#particle-variable-
 */

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
SYSTEM_MODE(AUTOMATIC); // Connect to Particle Cloud

int analogvalue = 0;
int loop_cnt = 0;
char *string = "Hello world!";

void setup() {
  Serial.begin(115200);
  Serial.println("Register cloud variables.");
  
  // variable name max length is 12 characters long
  if (Particle.variable("analog", analogvalue) == false) {
    Serial.println("variable 'analog' not registered!");
  }
  if (Particle.variable("counter", loop_cnt) == false) {
    Serial.println("variable 'counter' not registered!");
  }
  if (Particle.variable("greet", string) == false) {
    Serial.println("variable 'greet' not registered!");
  }

  pinMode(A0, INPUT);
}

void loop() {
  analogvalue = analogRead(A0);
  delay(3000);
  
  loop_cnt++;
  Serial.println(loop_cnt, DEC);
}

