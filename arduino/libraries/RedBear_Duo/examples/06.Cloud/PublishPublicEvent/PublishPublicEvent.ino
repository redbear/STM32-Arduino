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
 * Anyone may subscribe to PUBLIC events, thinking of them like tweets. Thus, the event you published as public
 * may be subscribed by other users and vice versa. You can sit in office to subscribe the events published by your Duo,
 * wherever your Duo is, as long as it is connecting to the Particle Cloud.
 * 
 * Subscribe public events using curl:
 *     curl -H "Authorization: Bearer {YOUR_ACCESS_TOKEN}" https://api.particle.io/v1/events/{EVENT_NAME} -k
 * E.g.(assume that your access token is 12345678901234567890):
 *     1. curl -H "Authorization: Bearer 12345678901234567890" https://api.particle.io/v1/events/arduino-start -k
 *     2. curl -H "Authorization: Bearer 12345678901234567890" https://api.particle.io/v1/events/button-pressed -k
 *     3. curl -H "Authorization: Bearer 12345678901234567890" https://api.particle.io/v1/events/periodic-event -k
 * Maybe someone else is publishing their public temperature event, you can also subscribe it even if your Duo is not online:
 *     4. curl -H "Authorization: Bearer 12345678901234567890" https://api.particle.io/v1/events/temperature -k
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

int button = D0;
unsigned long ms = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("Application started!");

  pinMode(button, INPUT_PULLUP);

  // Publish an event without data, with default TTL(time to live) 60 seconds.
  // Event name: 1–63 ASCII characters
  if (!Particle.publish("Arduino-start")) { 
    Serial.println("Publish 'arduino-start' event failed.");
  }
  Serial.println("Publish 'arduino-start' event successfully.");

  ms = millis();
}

void loop() {
  // Publish an event with data, with user specified TTL.
  if (digitalRead(button) == LOW) {
    delay(50); // Debounce
    if (digitalRead(button) == LOW) {
      if (!Particle.publish("button-pressed", "LOW", 21600)) {
        Serial.println("Publish 'button-pressed' event failed.");
      }
      Serial.println("Publish 'button-pressed' event successfully.");
      while (digitalRead(button) == LOW); // Wait to release.
    }
  }

  if ((millis() - ms) >= 5000) {
    ms = millis();
    // Publish an event with data, with default TTL 60 seconds.
    if (!Particle.publish("periodic-event", "5 seconds period event")) { 
      Serial.println("Publish 'periodic-event' event failed.");
    }
    Serial.println("Publish 'periodic-event' event successfully.");
  }
}

