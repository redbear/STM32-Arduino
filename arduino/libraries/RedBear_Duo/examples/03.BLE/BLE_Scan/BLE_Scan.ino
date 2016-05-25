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
 
#if defined(ARDUINO) 
SYSTEM_MODE(MANUAL);//do not connect to cloud
#else
SYSTEM_MODE(AUTOMATIC);//connect to cloud
#endif

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
  ble.startScanning();
  Serial.println("BLE scan start.");
}

void loop() {

}

