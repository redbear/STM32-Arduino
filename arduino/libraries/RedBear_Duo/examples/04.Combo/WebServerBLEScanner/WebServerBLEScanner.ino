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
 
#include "MDNS.h"

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

#define MAX_BLE_REPORT  20

static btstack_timer_source_t scan_timer;
static btstack_timer_source_t client_timer;

static uint8_t scan_flag = 1;

uint8_t adveventtype_temp[MAX_BLE_REPORT] = {};
uint8_t peeraddrtype_temp[MAX_BLE_REPORT] = {};                                
uint8_t peeraddr_temp[MAX_BLE_REPORT][6] = {};
int rssi_temp[MAX_BLE_REPORT] = {};
uint8_t advdatalen_temp[MAX_BLE_REPORT] = {};
uint8_t advdata_temp[MAX_BLE_REPORT][100] = {};                                 

static uint8_t ble_report_count = 0;

TCPServer server = TCPServer(80);
TCPClient client;
MDNS mdns;

void mdns_init();
void printWifiStatus();

void mdns_init() {
  bool success = mdns.setHostname("duo");
  if (success) {
    success = mdns.setService("tcp", "duosample", 80, "Duo Example Web");
    Serial.println("setService");
    if (success) {
      success = mdns.begin();
      Serial.println("mdns.begin");
      if (success) {
        Spark.publish("mdns/setup", "success");
        Serial.println("mdns/setup success");
        return;
      } 
    }
  }

  Spark.publish("mdns/setup", "error");
  Serial.println("mdns/setup error");
}

void reportCallback(advertisementReport_t *report) {
  static uint8_t same_flag = 1;
    
  for (int i = 0; i < ble_report_count; i++) {
//    Serial.println("i: ");
//    Serial.println(i );
    same_flag = 1;
    for (uint8_t h = 0; h < 6; h++) {
      if (peeraddr_temp[i][h] != report->peerAddr[h]) {
        same_flag = 0;
        //Serial.println("not same device 1");
        break;
      }          
    }
        
    if (same_flag == 1) {
      //Serial.println("same_flag=1");
      return;
    }
  }
    
  if (ble_report_count == 0) {
    same_flag=0;
    //Serial.println("same flag=0");
  }
 
  Serial.print("ble_report_count:  ");
  Serial.println(ble_report_count);
    
  Serial.println("reportCallback: ");
  Serial.print("The advEventType: ");
  adveventtype_temp[ble_report_count] = report->advEventType;
  Serial.println(adveventtype_temp[ble_report_count], HEX);
  Serial.print("The peerAddrType: ");
  peeraddrtype_temp[ble_report_count] = report->peerAddrType;
  Serial.println(peeraddrtype_temp[ble_report_count], HEX);
  Serial.print("The peerAddr: ");
  memcpy(peeraddr_temp[ble_report_count], report->peerAddr, 6);
  for (uint8_t index = 0; index < 6; index++) {
    Serial.print(peeraddr_temp[ble_report_count][index], HEX);
    Serial.print(" ");
  }
  Serial.println(" ");
    
  Serial.print("The rssi: ");
  rssi_temp[ble_report_count] = report->rssi;
  Serial.println(rssi_temp[ble_report_count], DEC);
    
  Serial.print("The ADV data: ");
  advdatalen_temp[ble_report_count] = report->advDataLen;
  memcpy(advdata_temp[ble_report_count], report->advData, advdatalen_temp[ble_report_count]);
  for (uint8_t index = 0; index < advdatalen_temp[ble_report_count]; index++) {
    Serial.print(advdata_temp[ble_report_count][index], HEX);
    Serial.print(" ");
  }
  Serial.println(" ");
  Serial.println(" ");
        
  ble_report_count++;
}

static void  scan_timer_intrp(btstack_timer_source_t *ts) {
  scan_flag = !scan_flag;
    
  if (scan_flag == 1) {
    Serial.println("ble startScanning");
    //ble.setScanParams(0, 0x00C0, 0x0030);
    memset(adveventtype_temp, 0, sizeof(adveventtype_temp));
    memset(peeraddrtype_temp, 0, sizeof(peeraddrtype_temp));
    memset(peeraddr_temp, 0, sizeof(peeraddr_temp));                
    memset(rssi_temp, 0, sizeof(rssi_temp)); 
    memset(advdatalen_temp, 0, sizeof(advdatalen_temp)); 
    memset(advdata_temp, 0, sizeof(advdata_temp));  
    ble.startScanning();
  }
  else {
    Serial.println("ble stopScanning");
    ble.stopScanning();
  }
  ble_report_count = 0;
  // reset
  ble.setTimer(ts, 10000);
  ble.addTimer(ts);
}

static void  client_timer_intrp(btstack_timer_source_t *ts) {
  if (scan_flag == 0) {
    // listen for incoming clients
    TCPClient client = server.available();
    if (client) {
      Serial.println("new client");
      // an http request ends with a blank line
      boolean currentLineIsBlank = true;
      while (client.connected()) {
        if (client.available()) {
          char c = client.read();
          Serial.write(c);
          // if you've gotten to the end of the line (received a newline
          // character) and the line is blank, the http request has ended,
          // so you can send a reply
          if (c == '\n' && currentLineIsBlank) {
            // send a standard http response header
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/html");
            client.println("Connection: close");  // the connection will be closed after completion of the response
            client.println("Refresh: 3");  // refresh the page automatically every 4 sec
            client.println();
            client.println("<!DOCTYPE HTML>");
            client.println("<html>");
  
            for (uint8_t i = 0; i < MAX_BLE_REPORT; i++) {
              if (rssi_temp[i] != 0) {
                client.println("reportCallback: ");
                client.println(i);
                client.println("<br />");
                client.print("The advEventType: ");
                client.println(adveventtype_temp[i], HEX);
                client.println("<br />");
                client.print("The peerAddrType: ");
                client.println(peeraddrtype_temp[i], HEX);
                client.println("<br />");
                client.print("The peerAddr: ");
                for (uint8_t index = 0; index < 6; index++) {
                  client.print(peeraddr_temp[i][index], HEX);
                  client.print(" ");
                }
                client.println(" ");
                client.println("<br />");
                client.print("The rssi: ");
                client.println(rssi_temp[i], DEC);
                client.println("<br />");
                client.print("The ADV data: ");
                for (uint8_t index = 0; index < advdatalen_temp[i]; index++) {
                  client.print(advdata_temp[i][index], HEX);
                  client.print(" ");
                }
                client.println(" ");
                client.println(" ");
                client.println("<br />");
                client.println(" ");
                client.println("<br />");
              }
            }
            client.println("</html>");
            break;
          }
          if (c == '\n') {
            // you're starting a new line
            currentLineIsBlank = true;
          }
          else if (c != '\r') {
            // you've gotten a character on the current line
            currentLineIsBlank = false;
          }
        }
      }
      // give the web browser time to receive the data
      delay(10);
      // close the connection:
      client.stop();
      Serial.println("client disonnected");  
    }
  }
  // reset
  ble.setTimer(ts, 500);
  ble.addTimer(ts);
}

void setup() {
  char addr[16];
    
  Serial.begin(115200);
  delay(5000);
  Serial.println("BLE scan web demo.");
  
  WiFi.on();
  WiFi.connect();
  
  IPAddress localIP = WiFi.localIP();
  while (localIP[0] == 0) {
    localIP = WiFi.localIP();
    Serial.println("waiting for an IP address");
    delay(1000);
  }

  // you're connected now, so print out the status:
  printWifiStatus();
  
  server.begin();

  mdns_init();

  ble.init();

  ble.onScanReportCallback(reportCallback);
  //ble.setScanParams(0, 0x00C0, 0x0030);
  ble.startScanning();

  // set one-shot timer
  scan_timer.process = &scan_timer_intrp;
  ble.setTimer(&scan_timer, 10000);//2000ms
  ble.addTimer(&scan_timer);

  // set one-shot timer
  client_timer.process = &client_timer_intrp;
  ble.setTimer(&client_timer, 500);//2000ms
  ble.addTimer(&client_timer);
    
  Serial.println("BLE scan start.");
}

void loop() {
  mdns.processQueries();
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

