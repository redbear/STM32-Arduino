#if defined(ARDUINO) 
SYSTEM_MODE(MANUAL);//do not connect to cloud
#else
SYSTEM_MODE(AUTOMATIC);//connect to cloud
#endif

#define DEVICE_NAME                "BLE_Peripheral"

#define CHARACTERISTIC1_MAX_LEN    8
#define CHARACTERISTIC2_MAX_LEN    8
#define CHARACTERISTIC3_MAX_LEN    8

static uint8_t service1_uuid[16] ={0x71,0x3d,0x00,0x00,0x50,0x3e,0x4c,0x75,0xba,0x94,0x31,0x48,0xf1,0x8d,0x94,0x1e};
static uint8_t char1_uuid[16]    ={0x71,0x3d,0x00,0x02,0x50,0x3e,0x4c,0x75,0xba,0x94,0x31,0x48,0xf1,0x8d,0x94,0x1e};
static uint8_t char2_uuid[16]    ={0x71,0x3d,0x00,0x03,0x50,0x3e,0x4c,0x75,0xba,0x94,0x31,0x48,0xf1,0x8d,0x94,0x1e};

static uint8_t service2_uuid[16] ={0x71,0x3d,0x00,0x00,0x51,0x3e,0x4c,0x75,0xba,0x94,0x31,0x48,0xf1,0x8d,0x94,0x1e};
static uint8_t char3_uuid[16]    ={0x71,0x3d,0x00,0x02,0x51,0x3e,0x4c,0x75,0xba,0x94,0x31,0x48,0xf1,0x8d,0x94,0x1e};


static uint8_t  appearance[2]    = {0x00, 0x02};
static uint8_t  change[2]        = {0x00, 0x00};
static uint8_t  conn_param[8]    = {0x28, 0x00, 0x90, 0x01, 0x00, 0x00, 0x90, 0x01};

static uint16_t character1_handle = 0x0000;
static uint16_t character2_handle = 0x0000;
static uint16_t character3_handle = 0x0000;

static uint8_t characteristic1_data[CHARACTERISTIC1_MAX_LEN]={0x01};
static uint8_t characteristic2_data[CHARACTERISTIC2_MAX_LEN]={0x00};
static uint8_t characteristic3_data[CHARACTERISTIC2_MAX_LEN]={0x03};

static hal_timer_source_t characteristic2;

static advParams_t adv_params;
static uint8_t adv_data[]={0x02,0x01,0x1A, 0x11,0x07,0x1e,0x94,0x8d,0xf1,0x48,0x31,0x94,0xba,0x75,0x4c,0x3e,0x50,0x00,0x00,0x3d,0x71};


void deviceConnectedCallback(BLEStatus_t status, uint16_t handle) {
    switch (status){
        case BLE_STATUS_OK:
            Serial.println("Device connected!");
            break;
        default:
            break;
    }
}

void deviceDisconnectedCallback(uint16_t handle){
    Serial.println("Disconnected.");
}

uint16_t gattReadCallback(uint16_t value_handle, uint8_t * buffer, uint16_t buffer_size)
{
    uint8_t characteristic_len=0;

    Serial.print("Read value handler: ");
    Serial.println(value_handle, HEX);

    if(character1_handle == value_handle)
    {
        Serial.println("Character1 read:");
        memcpy(buffer, characteristic1_data, CHARACTERISTIC1_MAX_LEN);
        characteristic_len = CHARACTERISTIC1_MAX_LEN;
    }
    else if(character1_handle+1 == value_handle)
    {
        Serial.println("Character1 cccd read:");
        uint8_t buf[2]={0x01,0x00};
        memcpy(buffer, buf, 2);
        characteristic_len = 2;
    }
    else if(character2_handle == value_handle)
    {
        Serial.println("Character2 read:");
        memcpy(buffer, characteristic2_data, CHARACTERISTIC2_MAX_LEN);
        characteristic_len = CHARACTERISTIC2_MAX_LEN;
    }
    return characteristic_len;
}

int gattWriteCallback(uint16_t value_handle, uint8_t *buffer, uint16_t size)
{
    Serial.print("Write value handler: ");
    Serial.println(value_handle, HEX);

    if(character1_handle == value_handle)
    {
        memcpy(characteristic1_data, buffer, size);
        Serial.print("Characteristic1 write value: ");
        for(uint8_t index=0; index<size; index++)
        {
            Serial.print(characteristic1_data[index], HEX);
            Serial.print(" ");
        }
        Serial.println(" ");
    }
    else if(character1_handle+1 == value_handle)
    {
        Serial.print("Characteristic1 cccd write value: ");
        for(uint8_t index=0; index<size; index++)
        {
            Serial.print(buffer[index], HEX);
            Serial.print(" ");
        }
        Serial.println(" ");
    }
    else if(character2_handle == value_handle)
    {
        memcpy(characteristic2_data, buffer, size);
        Serial.print("Characteristic2 write value: ");
        for(uint8_t index=0; index<size; index++)
        {
            Serial.print(characteristic2_data[index], HEX);
            Serial.print(" ");
        }
        Serial.println(" ");
    }
    return 0;
}

static void  characteristic2_notify(struct hal_timer *ts)
{
    Serial.println("characteristic2_notify");

    characteristic2_data[CHARACTERISTIC2_MAX_LEN-1]++;
    ble.sendNotify(character2_handle, characteristic2_data, CHARACTERISTIC2_MAX_LEN);
    // reset
    ble.setTimer(ts, 10000);
    ble.addTimerToLoop(ts);
}

void setup()
{
    Serial.begin(115200);
    delay(5000);
    Serial.println("BLE peripheral demo.");
    //ble.debugLogger(true);
    ble.init();

    ble.onConnectedCallback(deviceConnectedCallback);
    ble.onDisconnectedCallback(deviceDisconnectedCallback);
    ble.onDataReadCallback(gattReadCallback);
    ble.onDataWriteCallback(gattWriteCallback);

    ble.addService(0x1800);
    ble.addCharacteristic(0x2A00, PROPERTY_READ|PROPERTY_WRITE, (uint8_t*)DEVICE_NAME, sizeof(DEVICE_NAME));
    ble.addCharacteristic(0x2A01, PROPERTY_READ, appearance, sizeof(appearance));
    ble.addCharacteristic(0x2A04, PROPERTY_READ, conn_param, sizeof(conn_param));
    ble.addService(0x1801);
    ble.addCharacteristic(0x2A05, PROPERTY_INDICATE, change, sizeof(change));

    ble.addService(service1_uuid);
    character1_handle = ble.addCharacteristicDynamic(char1_uuid, PROPERTY_NOTIFY|PROPERTY_WRITE_WITHOUT_RESPONSE, characteristic1_data, CHARACTERISTIC1_MAX_LEN);
    character2_handle = ble.addCharacteristicDynamic(char2_uuid, PROPERTY_READ|PROPERTY_NOTIFY, characteristic2_data, CHARACTERISTIC2_MAX_LEN);
    ble.addService(service2_uuid);
    character3_handle = ble.addCharacteristic(char1_uuid, PROPERTY_READ, characteristic3_data, CHARACTERISTIC3_MAX_LEN);
    
    adv_params.adv_int_min = 0x0030;
    adv_params.adv_int_max = 0x0030;
    adv_params.adv_type    = 0;
    adv_params.dir_addr_type = 0;
    memset(adv_params.dir_addr,0,6);
    adv_params.channel_map = 0x07;
    adv_params.filter_policy = 0x00;
    
    ble.setAdvParams(&adv_params);
    
    ble.setAdvData(sizeof(adv_data), adv_data);
    
    ble.startAdvertising();
    Serial.println("BLE start advertising.");
    
    // set one-shot timer
    characteristic2.process = &characteristic2_notify;
    ble.setTimer(&characteristic2, 10000);//2000ms
    ble.addTimerToLoop(&characteristic2);
}

void loop()
{
    ble.loop();
}

