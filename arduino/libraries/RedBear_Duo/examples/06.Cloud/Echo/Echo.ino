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

#define MAX_BUF_LEN    255 // Data in an event to be published to Particle Cloud limits to 255 bytes;

int button = D0;
uint8_t buf[MAX_BUF_LEN]; 
uint8_t buf_len = 0;

void btnEvtHandler(const char *event, const char *data);
void serialEvtHandler(const char *event, const char *data);

void setup() {
  Serial.begin(115200);
  Serial.println("Application started!");

  pinMode(button, INPUT_PULLUP);

  if(!Particle.subscribe("button-pressed", btnEvtHandler)) {
    Serial.println("Subscribe to event 'button-pressed' failed.");
  }
  Serial.println("Subscribe to event 'button-pressed' successfully.");

  if(!Particle.subscribe("serial-event", serialEvtHandler)) {
    Serial.println("Subscribe to event 'serial-event' failed.");
  }
  Serial.println("Subscribe to event 'serial-event' successfully.");
}

void loop() {
  if(digitalRead(button) == LOW) {
    delay(50); // Debounce
    if(digitalRead(button) == LOW) {
      if (!Particle.publish("button-pressed", "LOW")) {
        Serial.println("Publish 'button-pressed' event failed.");
      }
      Serial.println("Publish 'button-pressed' event successfully.");
      
      while(digitalRead(button) == LOW); // Wait to release.
      
      if (!Particle.publish("button-pressed", "HIGH")) {
        Serial.println("Publish 'button-pressed' event failed.");
      }
      Serial.println("Publish 'button-pressed' event successfully.");
    }
  }

  if(Serial.available()) {
    while(Serial.available()) {
      buf[buf_len++] = Serial.read();
      if(buf_len >= (MAX_BUF_LEN-1)) break;
    }
    buf[buf_len] = '\0';
    if (!Particle.publish("serial-event", (const char *)buf)) {
      Serial.println("serial-event' event failed.");
    }
    Serial.println("Publish 'serial-event' event successfully.");
    buf_len = 0;
  }
}

void btnEvtHandler(const char *event, const char *data) {
  Serial.print(event);
  Serial.print(", data: ");
  if (data)
    Serial.println(data);
  else
    Serial.println("NULL");
}

void serialEvtHandler(const char *event, const char *data) {
  Serial.print(event);
  Serial.print(", data: ");
  if (data)
    Serial.println(data);
  else
    Serial.println("NULL");
}

