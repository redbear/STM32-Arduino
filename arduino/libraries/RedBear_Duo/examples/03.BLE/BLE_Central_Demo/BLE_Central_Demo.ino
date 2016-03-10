
SYSTEM_MODE(MANUAL);

static uint16_t connected_id = 0xFFFF;

typedef struct {
    uint16_t  connected_handle;
    uint8_t   addr_type;
    bd_addr_t addr;
    struct {
        gatt_client_service_t service;
        struct{
            gatt_client_characteristic_t chars;
            gatt_client_characteristic_descriptor_t descriptor[2];
        }chars[2];
    }service;
}Device_t;

Device_t device;
uint8_t  chars_index=0;
uint8_t  desc_index=0;

static uint8_t service1_uuid[16] ={0x71,0x3d,0x00,0x00,0x50,0x3e,0x4c,0x75,0xba,0x94,0x31,0x48,0xf1,0x8d,0x94,0x1e};

static uint8_t gatt_notify_flag=0;

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
            device.addr_type = report->peerAddrType;
            memcpy(device.addr, report->peerAddr, 6);

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
            device.connected_handle = handle;
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
        Serial.println("Restart scanning.");
        connected_id = 0xFFFF;
        ble.startScanning();
    }
}

static void discoveredServiceCallback(BLEStatus_t status, uint16_t con_handle, gatt_client_service_t *service)
{
    if(status == BLE_STATUS_OK)
    {
        Serial.println(" ");
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
        if(0x00 == memcmp(service->uuid128, service1_uuid, 16))
        {
            Serial.println("Target uuid128");
            device.service.service = *service;
        }
    }
    else if(status == BLE_STATUS_DONE)
    {
        Serial.println("Discovered service done");
        ble.discoverCharacteristics(device.connected_handle, &device.service.service);
    }
}


static void discoveredCharsCallback(BLEStatus_t status, uint16_t con_handle, gatt_client_characteristic_t *characteristic)
{
    if(status == BLE_STATUS_OK)
    {
        Serial.println(" ");
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
        if(chars_index < 2)
        { 
            device.service.chars[chars_index].chars= *characteristic;
            chars_index++;
        }
    }
    else if(status == BLE_STATUS_DONE)
    {
        Serial.println("Discovered characteristic done");
        chars_index = 0;
        ble.discoverCharacteristicDescriptors(device.connected_handle, &device.service.chars[chars_index].chars);
    }
}

static void discoveredCharsDescriptorsCallback(BLEStatus_t status, uint16_t con_handle, gatt_client_characteristic_descriptor_t *descriptor)
{
    if(status == BLE_STATUS_OK)
    {
        Serial.println(" ");
        Serial.print("descriptor handle: ");
        Serial.println(descriptor->handle, HEX);
        Serial.print("descriptor uuid16: ");
        Serial.println(descriptor->uuid16, HEX);
        uint8_t index;
        Serial.print("descriptor uuid128 : ");
        for(index=0; index<16; index++)
        {
            Serial.print(descriptor->uuid128[index], HEX);
            Serial.print(" ");
        }
        Serial.println(" ");
        if(desc_index < 2)
        {
            device.service.chars[chars_index].descriptor[desc_index++] = *descriptor;
        }
    }
    else if(status == BLE_STATUS_DONE)
    {
        Serial.println("Discovered descriptor done");
        chars_index++;
        if(chars_index < 2)
        {
            desc_index=0;
            ble.discoverCharacteristicDescriptors(device.connected_handle, &device.service.chars[chars_index].chars);
        }
        else
        {
            ble.readValue(device.connected_handle,&device.service.chars[1].chars);
        }
    }
}


void gattReadCallback(BLEStatus_t status, uint16_t con_handle, uint16_t value_handle, uint8_t *value, uint16_t length)
{
    if(status == BLE_STATUS_OK)
    {
        Serial.println(" ");
        Serial.println("Read characteristic ok");
        Serial.print("conn handle: ");
        Serial.println(con_handle, HEX);
        Serial.print("value handle: ");
        Serial.println(value_handle, HEX);
        uint8_t index;
        Serial.print("The value : ");
        for(index=0; index<length; index++)
        {
            Serial.print(value[index], HEX);
            Serial.print(" ");
        }
        Serial.println(" ");
    }
    else if(status == BLE_STATUS_DONE)
    {
        uint8_t data[]= {0x01,0x02,0x03,0x04,0x05,1,2,3,4,5};
        ble.writeValue(device.connected_handle, device.service.chars[0].chars.value_handle, sizeof(data), data);
    }
}

void gattWrittenCallback(BLEStatus_t status, uint16_t con_handle)
{
    if(BLE_STATUS_DONE)
    {
        Serial.println(" ");
        Serial.println("Write characteristic done");
        ble.readDescriptorValue(device.connected_handle, device.service.chars[0].descriptor[0].handle);
    }
}

void gattReadDescriptorCallback(BLEStatus_t status, uint16_t con_handle, uint16_t value_handle, uint8_t *value, uint16_t length)
{
    if(status == BLE_STATUS_OK)
    {
        Serial.println(" ");
        Serial.println("Read descriptor ok");
        Serial.print("conn handle: ");
        Serial.println(con_handle, HEX);
        Serial.print("value handle: ");
        Serial.println(value_handle, HEX);
        uint8_t index;
        Serial.print("The value : ");
        for(index=0; index<length; index++)
        {
            Serial.print(value[index], HEX);
            Serial.print(" ");
        }
        Serial.println(" ");
    }
    else if(status == BLE_STATUS_DONE)
    {
        ble.writeClientCharsConfigDescritpor(device.connected_handle, &device.service.chars[0].chars, GATT_CLIENT_CHARACTERISTICS_CONFIGURATION_NOTIFICATION);
    }
}

void gattWriteCCCDCallback(BLEStatus_t status, uint16_t con_handle)
{
    if(status == BLE_STATUS_DONE)
    {
        Serial.println("gattWriteCCCDCallback done");
        if(gatt_notify_flag == 0)
        { 
            gatt_notify_flag = 1;
            ble.writeClientCharsConfigDescritpor(device.connected_handle, &device.service.chars[1].chars, GATT_CLIENT_CHARACTERISTICS_CONFIGURATION_NOTIFICATION);
        }
        else if(gatt_notify_flag == 1)
        {
            gatt_notify_flag = 2;
        }
    }
}

void gattNotifyUpdateCallback(BLEStatus_t status, uint16_t con_handle, uint16_t value_handle, uint8_t *value, uint16_t length)
{
    Serial.println(" ");
    Serial.println("Notify Update value ");
    Serial.print("conn handle: ");
    Serial.println(con_handle, HEX);
    Serial.print("value handle: ");
    Serial.println(value_handle, HEX);
    uint8_t index;
    Serial.print("The value : ");
    for(index=0; index<length; index++)
    {
        Serial.print(value[index], HEX);
        Serial.print(" ");
    }
    Serial.println(" ");
}

void setup()
{
    Serial.begin(115200);
    delay(5000);

    //ble.debugLogger(true);
    //ble.debugError(true);
    //ble.enablePacketLogger();
    Serial.println("BLE central demo!");
    ble.init();
    ble.onConnectedCallback(deviceConnectedCallback);
    ble.onDisconnectedCallback(deviceDisconnectedCallback);
    ble.onScanReportCallback(reportCallback);

    ble.onServiceDiscoveredCallback(discoveredServiceCallback);
    ble.onCharacteristicDiscoveredCallback(discoveredCharsCallback);
    ble.onDescriptorDiscoveredCallback(discoveredCharsDescriptorsCallback);
    ble.onGattCharacteristicReadCallback(gattReadCallback);
    ble.onGattCharacteristicWrittenCallback(gattWrittenCallback);
    ble.onGattDescriptorReadCallback(gattReadDescriptorCallback);

    ble.onGattWriteClientCharacteristicConfigCallback(gattWriteCCCDCallback);
    ble.onGattNotifyUpdateCallback(gattNotifyUpdateCallback);


    ble.setScanParams(0, 0x0030, 0x0030);
    ble.startScanning();
    Serial.println("Start scanning ");
}

void loop()
{

}


