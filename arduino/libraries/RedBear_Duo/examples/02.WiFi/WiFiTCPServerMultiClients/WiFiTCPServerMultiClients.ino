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
 *  This is a simple TCP server, telnet to this sketch with an IP assigned for Duo.
 *  e.g. telnet 192.168.0.5 8888
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
#if defined(ARDUINO) 
SYSTEM_MODE(SEMI_AUTOMATIC); 
#endif
 
/* Modify the bellowing definitions for your AP/Router.*/
// #define SSID     "YOUR_ROUTER_SSID"
// #define PASSWORD "YOUR_ROUTER_PASSWORD"

#define MAX_CLIENT_NUM   3

TCPServer server = TCPServer(8888);
TCPClient client[MAX_CLIENT_NUM];
    
void setup() {
  char addr[16];

  Serial.begin(115200);
  delay(5000);
    
  WiFi.on();
  WiFi.setCredentials(SSID, PASSWORD, WPA2);
  WiFi.connect();
  
  Serial.println("Waiting for an IP address...\n");
  while (!WiFi.ready()) {
    delay(1000);
  }
  
  // Wait IP address to be updated.
  IPAddress localIP = WiFi.localIP();
  while (localIP[0] == 0) {
    localIP = WiFi.localIP();
    delay(1000);
  }
  
  sprintf(addr, "%u.%u.%u.%u", localIP[0], localIP[1], localIP[2], localIP[3]);
  Serial.println(addr);

  server.begin();
}
    
void loop() {
  for(uint8_t i = 0; i < MAX_CLIENT_NUM; i++) {
    if (client[i].connected()) {
      Serial.println("Connected by TCP client.");
      client[i].println("Hello!");
    }
    else {
      client[i] = server.available();
    }
  }

  delay(2000);
}

