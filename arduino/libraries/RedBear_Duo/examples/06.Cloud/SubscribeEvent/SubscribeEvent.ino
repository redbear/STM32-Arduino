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
 * Subscribe to events published by devices.This allows devices to talk to each other very easily. For example, one device 
 * could publish events when a motion sensor is triggered and another could subscribe to these events and respond by sounding an alarm.
 * 
 * A subscription works like a prefix filter. If you subscribe to "foo", you will receive any public event or 
 * your device's private event whose name begins with "foo", including "foo", "fool", "foobar", and "food/indian/sweet-curry-beans". 
 * 
 * It is ok to register a subscription when the device is not connected to the cloud - the subscription 
 * is automatically registered with the cloud next time the device connects.
 * 
 * A device can register up to 4 event handlers. This means you can call Particle.subscribe() a maximum of 4 times; after that it will return false.
 * 
 * To use Particle.subscribe(), define a handler function and register it in setup().
 * 
 * Learn more about subscribing events: https://docs.particle.io/reference/firmware/photon/#particle-subscribe-
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
int humidity_event_cnt = 0;

void allTemperEvtHandler(const char *event, const char *data);
void myHumidityEvtHandler(const char *event, const char *data);

void setup() {
  Serial.begin(115200);
  Serial.println("Application started.");

  if (!Particle.subscribe("temperature", allTemperEvtHandler)) {
    Serial.println("Subscribe to event 'temperature' failed.");
  }
  Serial.println("Subscribe to event 'temperature' successfully.");

  // You can listen to events published only by your own devices by adding a MY_DEVICES constant.
  if (!Particle.subscribe("humidity", myHumidityEvtHandler, MY_DEVICES)) {
    Serial.println("Subscribe to event 'humidity' failed.");
  }
  Serial.println("Subscribe to event 'humidity' successfully.");
}

void loop() {
  // Do nothing.
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

void myHumidityEvtHandler(const char *event, const char *data) {
  humidity_event_cnt++;
  Serial.print(humidity_event_cnt);
  Serial.print(" ");
  Serial.print(event);
  Serial.print(", data: ");
  if (data)
    Serial.println(data);
  else
    Serial.println("NULL");
}

