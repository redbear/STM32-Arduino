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
#if defined(ARDUINO) 
SYSTEM_MODE(SEMI_AUTOMATIC); 
#endif

/* 
 * BLE scan parameters:
 *     - BLE_SCAN_TYPE     
 *           0x00: Passive scanning, no scan request packets shall be sent.(default)
 *           0x01: Active scanning, scan request packets may be sent.
 *           0x02 - 0xFF: Reserved for future use.
 *     - BLE_SCAN_INTERVAL: This is defined as the time interval from when the Controller started its last LE scan until it begins the subsequent LE scan.
 *           Range: 0x0004 to 0x4000
 *           Default: 0x0010 (10 ms)
 *           Time = N * 0.625 msec
 *           Time Range: 2.5 msec to 10.24 seconds
 *     - BLE_SCAN_WINDOW: The duration of the LE scan. The scan window shall be less than or equal to the scan interval.
 *           Range: 0x0004 to 0x4000
 *           Default: 0x0010 (10 ms)
 *           Time = N * 0.625 msec
 *           Time Range: 2.5 msec to 10240 msec
 */
#define BLE_SCAN_TYPE        0x00   // Passive scanning
#define BLE_SCAN_INTERVAL    0x0060 // 60 ms
#define BLE_SCAN_WINDOW      0x0030 // 30 ms


void reportCallback(advertisementReport_t *report) {
  uint8_t index;

  Serial.println("reportCallback: ");
  Serial.print("The advEventType: ");
  Serial.println(report->advEventType, HEX);
  Serial.print("The peerAddrType: ");
  Serial.println(report->peerAddrType, HEX);
  Serial.print("The peerAddr: ");
  for (index = 0; index < 6; index++) {
    Serial.print(report->peerAddr[index], HEX);
    Serial.print(" ");
  }
  Serial.println(" ");

  Serial.print("The rssi: ");
  Serial.println(report->rssi, DEC);

  Serial.print("The ADV data: ");
  for (index = 0; index < report->advDataLen; index++) {
    Serial.print(report->advData[index], HEX);
    Serial.print(" ");
  }
  Serial.println(" ");
  Serial.println(" ");
}

void setup() {
  Serial.begin(115200);
  delay(5000);
  Serial.println("BLE scan demo.");
  
  ble.init();

  ble.onScanReportCallback(reportCallback);

  // Set scan parameters.
  ble.setScanParams(BLE_SCAN_TYPE, BLE_SCAN_INTERVAL, BLE_SCAN_WINDOW);
  
  ble.startScanning();
  Serial.println("BLE scan start.");
}

void loop() {

}

