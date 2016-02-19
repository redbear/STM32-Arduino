#if defined(ARDUINO) 
SYSTEM_MODE(MANUAL);//do not connect to cloud
#else
SYSTEM_MODE(AUTOMATIC);//connect to cloud
#endif

void reportCallback(advertisementReport_t *report)
{
    uint8_t index;

    Serial.println("reportCallback: ");
    Serial.print("The advEventType: ");
    Serial.println(report->advEventType, HEX);
    Serial.print("The peerAddrType: ");
    Serial.println(report->peerAddrType, HEX);
    Serial.print("The peerAddr: ");
    for(index=0; index<6; index++)
    {
        Serial.print(report->peerAddr[index], HEX);
        Serial.print(" ");
    }
    Serial.println(" ");

    Serial.print("The rssi: ");
    Serial.println(report->rssi, DEC);

    Serial.print("The ADV data: ");
    for(index=0; index<report->advDataLen; index++)
    {
        Serial.print(report->advData[index], HEX);
        Serial.print(" ");
    }
    Serial.println(" ");
}

void setup()
{
    Serial.begin(115200);
    delay(5000);
    Serial.println("BLE scan demo.");
    ble.init();

    ble.onScanReportCallback(reportCallback);
    ble.startScanning();
    Serial.println("BLE scan start.");
}

void loop()
{
    ble.loop();
}

