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
 * Publish an event through the Particle Cloud that will be forwarded to all registered listeners, such as 
 * callbacks, subscribed streams of Server-Sent Events, and other devices listening via Particle.subscribe().
 * Only the owner of the device will be able to subscribe to private events. You can sit in office to subscribe 
 * the private events published by your Duo, wherever your Duo is, as long as it is connecting to the Particle Cloud.
 * 
 * Subscribe private events using curl:
 *     curl -H "Authorization: Bearer {YOUR_ACCESS_TOKEN}" https://api.particle.io/v1/devices/events/{EVENT_NAME} -k
 * E.g.(assume that your access token is 12345678901234567890):
 *     1. curl -H "Authorization: Bearer 12345678901234567890" https://api.particle.io/v1/devices/events/human-detected -k
 *     2. curl -H "Authorization: Bearer 12345678901234567890" https://api.particle.io/v1/devices/events/my-temperature -k
 *     
 * You can also check the events on Particle Dashboard: https://dashboard.particle.io/user/logs
 *     
 * A subscription works like a prefix filter. If you subscribe to "foo", you will receive any event whose name begins 
 * with "foo", including "foo", "fool", "foobar", and "food/indian/sweet-curry-beans".
 *     
 * Your access token can be found on Particle Build: https://build.particle.io, After you login in, check it under the "Settings" tag.
 * Learn more about publishing public events: https://docs.particle.io/reference/firmware/photon/#particle-publish-
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

int PIR = D0;
bool quiet = true;
unsigned long ms = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("Application started!");

  pinMode(PIR, INPUT_PULLUP);

  ms = millis();
}

void loop() {
  // Publish an event with data, with user specified TTL.
  if (digitalRead(PIR) == HIGH && quiet) {
    delay(50); // Debounce
    if (digitalRead(PIR) == HIGH) {
      if (!Particle.publish("human-detected", NULL, 60, PRIVATE)) {
        Serial.println("Publish 'human-detected' event failed.");
      }
      Serial.println("Publish 'human-detected' event successfully.");
      quiet = false;
    }
  }

  if (digitalRead(PIR) == LOW) {
    delay(50); // Debounce
    if (digitalRead(PIR) == LOW)
      quiet = true;
  }

  if ((millis() - ms) >= 5000) {
    ms = millis();
    // In order to publish a private event, you must pass all four parameters.
    if (!Particle.publish("my-temperature", "600 F", 60, PRIVATE)) { 
      Serial.println("Publish 'my-temperature' event failed.");
    }
    Serial.println("Publish 'my-temperature' event successfully.");
  }
}

