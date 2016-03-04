
SYSTEM_MODE(MANUAL);

static uint8_t  connected_flag = 0;
static uint16_t connected_id = 0xFFFF;

gatt_client_service_t rtx_service;
gatt_client_characteristic_t tx_chars;

uint32_t ble_advdata_decode(uint8_t type, uint8_t advdata_len, uint8_t *p_advdata, uint8_t *len, uint8_t *p_field_data)
{
    uint8_t index=0;
    uint8_t field_length, field_type;

    while(index<advdata_len)
    {
        field_length = p_advdata[index];
        field_type   = p_advdata[index+1];
        if(field_type == type)
        {
            memcpy(p_field_data, &p_advdata[index+2], (field_length-1));
            *len = field_length - 1;
            return 0;
        }
        index += field_length + 1;
    }
    return 1;
}

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

    uint8_t len;
    uint8_t adv_name[31];
    if(0x00 == ble_advdata_decode(0x08, report->advDataLen, report->advData, &len, adv_name))
    {
        Serial.print("  The length of Short Local Name : ");
        Serial.println(len, HEX);
        Serial.print("  The Short Local Name is        : ");
        Serial.println((const char *)adv_name);
        if(0x00 == memcmp(adv_name, "Biscuit", min(7, len)))
        {
            ble.stopScanning();
            ble.connect(report->peerAddr, BD_ADDR_TYPE_LE_RANDOM);
        }
    }
    else if(0x00 == ble_advdata_decode(0x09, report->advDataLen, report->advData, &len, adv_name))
    {
        Serial.print("  The length of Complete Local Name : ");
        Serial.println(len, HEX);
        Serial.print("  The Complete Local Name is        : ");
        Serial.println((const char *)adv_name);
        if(0x00 == memcmp(adv_name, "Heart Rate", min(7, len)))
        { 

        }
    }
}

void deviceConnectedCallback(BLEStatus_t status, uint16_t handle) {
    switch (status){
        case BLE_STATUS_OK:
            Serial.println("Device connected!");
            connected_id = handle;
            ble.discoverPrimaryServices(handle);
            break;
        default:
            break;
    }
}

void deviceDisconnectedCallback(uint16_t handle){
    Serial.print("Disconnected handle:");
    Serial.println(handle,HEX);
    if(connected_id == handle)
    {
        Serial.println("Disconnected22.");
        connected_id = 0xFFFF;
        ble.startScanning();
    }
}

static void discoveredCharsDescriptorsCallback(BLEStatus_t status, gatt_client_characteristic_descriptor_t *descriptor)
{
    if(status == BLE_STATUS_OK)
    {
        Serial.print("descriptor handle: ");
        Serial.println(descriptor->handle, HEX);
        Serial.print("characteristic uuid16: ");
        Serial.println(descriptor->uuid16, HEX);
        uint8_t index;
        Serial.print("characteristic uuid128 : ");
        for(index=0; index<16; index++)
        {
            Serial.print(descriptor->uuid128[index], HEX);
            Serial.print(" ");
        }
        Serial.println(" ");
    }
    else if(status == BLE_STATUS_DONE)
    {
        Serial.println("Discovered characteristic descriptor done");
    }
}


static void discoveredCharsForService(BLEStatus_t status, gatt_client_characteristic_t *characteristic)
{
    if(status == BLE_STATUS_OK)
    {
        Serial.print("characteristic start handle: ");
        Serial.println(characteristic->start_handle, HEX);
        Serial.print("characteristic value handle: ");
        Serial.println(characteristic->value_handle, HEX);
        Serial.print("characteristic end_handle: ");
        Serial.println(characteristic->end_handle, HEX);
        Serial.print("characteristic properties: ");
        Serial.println(characteristic->properties, HEX);
        Serial.print("characteristic uuid16: ");
        Serial.println(characteristic->uuid16, HEX);
        uint8_t index;
        Serial.print("characteristic uuid128 : ");
        for(index=0; index<16; index++)
        {
            Serial.print(characteristic->uuid128[index], HEX);
            Serial.print(" ");
        }
        Serial.println(" ");
        tx_chars = *characteristic;
    }
    else if(status == BLE_STATUS_DONE)
    {
        Serial.println("Discovered characteristic done");
        ble.discoverCharacteristicDescriptors(connected_id, &tx_chars);
    }
}

static void discoveredServiceCallback(BLEStatus_t status, gatt_client_service_t *service)
{
    if(status == BLE_STATUS_OK)
    {
        Serial.print("Service start handle: ");
        Serial.println(service->start_group_handle, HEX);
        Serial.print("Service end handle: ");
        Serial.println(service->end_group_handle, HEX);
        Serial.print("Service uuid16: ");
        Serial.println(service->uuid16, HEX);
        uint8_t index;
        Serial.print("The uuid128 : ");
        for(index=0; index<16; index++)
        {
            Serial.print(service->uuid128[index], HEX);
            Serial.print(" ");
        }
        Serial.println(" ");
        rtx_service = *service;
    }
    else if(status == BLE_STATUS_DONE)
    {
        Serial.println("Discovered service done");
        ble.discoverCharacteristicsForService(connected_id, &rtx_service);
    }
}

void setup()
{
    Serial.begin(115200);
    delay(5000);

    ble.debugLogger(true);
    //ble.enablePacketLogger();

    ble.init();
    ble.onConnectedCallback(deviceConnectedCallback);
    ble.onDisconnectedCallback(deviceDisconnectedCallback);
    ble.onScanReportCallback(reportCallback);
    ble.onServiceDiscoveredCallback(discoveredServiceCallback);
    ble.onCharacteristicDiscoveredCallback(discoveredCharsForService);
    ble.onCharsDescriptorDiscoveredCallback(discoveredCharsDescriptorsCallback);

    ble.setScanParams(0, 0x0030, 0x0030);
    ble.startScanning();
}

void loop()
{

}

