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
 * Removes all subscription handlers previously registered with Particle.subscribe().
 * 
 * Learn more about subscribing events: https://docs.particle.io/reference/firmware/photon/#particle-unsubscribe-
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

int temper_event_cnt = 0;
int button = D0;
bool subscribed = false;

void allTemperEvtHandler(const char *event, const char *data);

void setup() {
  Serial.begin(115200);
  Serial.println("Application started.");

  pinMode(button, INPUT_PULLUP);
}

void loop() {
  if(digitalRead(button) == LOW) {
    delay(50); // Debounce
    if(digitalRead(button) == LOW) {
      if(subscribed) {
        Serial.println("Unsubscribe all events.");
        Particle.unsubscribe();
        subscribed = false;
      }
      else {
        if(Particle.subscribe("temperature", allTemperEvtHandler)) {
          Serial.println("Subscribe to event 'temperature' successfully.");
          subscribed = true;
          temper_event_cnt = 0;
        }
        else {
          Serial.println("Subscribe to event 'temperature' failed.");
        }
      }
      while(digitalRead(button) == LOW); // Wait to release
    }
  }
}

void allTemperEvtHandler(const char *event, const char *data) {
  temper_event_cnt++;
  Serial.print(temper_event_cnt);
  Serial.print(" ");
  Serial.print(event);
  Serial.print(", data: ");
  if (data)
    Serial.println(data);
  else
    Serial.println("NULL");
}

