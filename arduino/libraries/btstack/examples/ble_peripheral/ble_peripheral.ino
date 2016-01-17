#include <BTstack.h>
#include <btstack_api.h>
#include "Arduino.h"

//SYSTEM_MODE(AUTOMATIC);
SYSTEM_MODE(MANUAL);

static char characteristic_data = 'H';

void deviceConnectedCallback(BLEStatus status, BLEDevice *device) {
    switch (status){
        case BLE_STATUS_OK:
            Serial.println("Device connected!");
            break;
        default:
            break;
    }
}

void deviceDisconnectedCallback(BLEDevice * device){
    Serial.println("Disconnected.");
}

uint16_t gattReadCallback(uint16_t value_handle, uint8_t * buffer, uint16_t buffer_size){
    if (buffer){
        Serial.print("gattReadCallback, value: ");
        Serial.println(characteristic_data, HEX);
        buffer[0] = characteristic_data;
    }
    return 1;
}

int gattWriteCallback(uint16_t value_handle, uint8_t *buffer, uint16_t size){
    characteristic_data = buffer[0];
    Serial.print("gattWriteCallback , value ");
    Serial.println(characteristic_data, HEX);
    return 0;
}


void setup() {

    pinMode(D7, OUTPUT);
    Serial.begin(115200);
    delay(5000);
    
    BTstack.enableDebugLogger();
    BTstack.enablePacketLogger();
    // set callbacks
    BTstack.setBLEDeviceConnectedCallback(deviceConnectedCallback);
    BTstack.setBLEDeviceDisconnectedCallback(deviceDisconnectedCallback);
    BTstack.setGATTCharacteristicRead(gattReadCallback);
    BTstack.setGATTCharacteristicWrite(gattWriteCallback);

    // setup GATT Database
    BTstack.addGATTService(new UUID("B8E06067-62AD-41BA-9231-206AE80AB551"));
    BTstack.addGATTCharacteristic(new UUID("f897177b-aee8-4767-8ecc-cc694fd5fcef"), ATT_PROPERTY_READ, "This is a String!");
    BTstack.addGATTCharacteristicDynamic(new UUID("f897177b-aee8-4767-8ecc-cc694fd5fce0"), ATT_PROPERTY_READ | ATT_PROPERTY_WRITE | ATT_PROPERTY_NOTIFY, 0);

    bd_addr_t dummy = { 1,2,3,4,5,6};
    BTstack.setPublicBdAddr(dummy);
    // startup Bluetooth and activate advertisements
    BTstack.setup();
    BTstack.startAdvertising();
    Serial.println("Start ");
}

uint32_t time1 = 5000;
uint32_t time2;

void loop() {
  // put your main code here, to run repeatedly:
    BTstack.loop();
}


