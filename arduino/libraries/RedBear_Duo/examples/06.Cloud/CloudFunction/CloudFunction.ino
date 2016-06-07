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
 * As long as your Duo is connecting to the Particle Cloud, it can expose a function through the Particle Cloud 
 * so that you can call it wherever you can access the internet.
 * 
 * Call the functions using curl:
 *     curl https://api.particle.io/v1/devices/{YOUR_DEVICE_ID}/{FUNCTION_NAME} -d access_token={YOUR_ACCESS_TOKEN} -d "args={ARGUMENTS}" -k
 * E.g.(Assume that your Duo's device ID is: 112233445566778899aabbcc and your access token is 12345678901234567890):
 *     1. curl https://api.particle.io/v1/devices/112233445566778899aabbcc/echo -d access_token=12345678901234567890 -d "args=Hello" -k
 *     2. curl https://api.particle.io/v1/devices/112233445566778899aabbcc/relay -d access_token=12345678901234567890 -d "args=ON" -k
 *   
 * Your access token can be found on Particle Build: https://build.particle.io, After you login in, check it under the "Settings" tag.
 * Learn more about cloud function: https://docs.particle.io/reference/firmware/photon/#particle-function-
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
SYSTEM_MODE(AUTOMATIC); // Connect to Particle Cloud to tinker your Duo.

int relay = D0;

int echoText(String text);
int controlRelay(String command);

void setup() {
  Serial.begin(115200);
  Serial.println("Application started.");

  pinMode(relay, OUTPUT);
  digitalWrite(relay, LOW);
  
  //Register all cloud functions
  if (!Particle.function("echo", echoText)) {
    Serial.println("Function 'echo' register failed.");
  }
  if (!Particle.function("relay", controlRelay)) {
    Serial.println("Function 'relay' register failed.");
  }
}

void loop() {
  // do nothing.
}

int echoText(String text) {
  Serial.print("Received text: ");
  Serial.println(text);
  return 1;
}

int controlRelay(String command) {
  bool value = 0;
  
  if (command.substring(0,2) == "ON") value = 1;
  else if (command.substring(0,3) == "OFF") value = 0;
  else return -1;

  digitalWrite(relay, value);
  Serial.print("Relay state: ");
  if (value == 1)
    Serial.println("ON");
  else
    Serial.println("OFF");
  
  return 1;
}

