
/******************************************************
 *                      Macros
 ******************************************************/
#if defined(ARDUINO) 
SYSTEM_MODE(MANUAL);//do not connect to cloud
#else
SYSTEM_MODE(AUTOMATIC);//connect to cloud
#endif

#define DEVICE_NAME                "BLE_Peripheral"

// Length of characteristic value.
#define CHARACTERISTIC1_MAX_LEN    8
#define CHARACTERISTIC2_MAX_LEN    8
#define CHARACTERISTIC3_MAX_LEN    8

/******************************************************
 *               Variable Definitions
 ******************************************************/
// Primary service 128-bits UUID
static uint8_t service1_uuid[16] ={0x71,0x3d,0x00,0x00,0x50,0x3e,0x4c,0x75,0xba,0x94,0x31,0x48,0xf1,0x8d,0x94,0x1e};
// Characteristics 128-bits UUID
static uint8_t char1_uuid[16]    ={0x71,0x3d,0x00,0x02,0x50,0x3e,0x4c,0x75,0xba,0x94,0x31,0x48,0xf1,0x8d,0x94,0x1e};
static uint8_t char2_uuid[16]    ={0x71,0x3d,0x00,0x03,0x50,0x3e,0x4c,0x75,0xba,0x94,0x31,0x48,0xf1,0x8d,0x94,0x1e};

// Primary service 128-bits UUID
static uint8_t service2_uuid[16] ={0x71,0x3d,0x00,0x00,0x51,0x3e,0x4c,0x75,0xba,0x94,0x31,0x48,0xf1,0x8d,0x94,0x1e};
// Characteristics 128-bits UUID
static uint8_t char3_uuid[16]    ={0x71,0x3d,0x00,0x02,0x51,0x3e,0x4c,0x75,0xba,0x94,0x31,0x48,0xf1,0x8d,0x94,0x1e};

static uint8_t  appearance[2]    = {0x00, 0x02};
static uint8_t  change[2]        = {0x00, 0x00};
static uint8_t  conn_param[8]    = {0x28, 0x00, 0x90, 0x01, 0x00, 0x00, 0x90, 0x01}; 

// Characteristic value handle
static uint16_t character1_handle = 0x0000;
static uint16_t character2_handle = 0x0000;
static uint16_t character3_handle = 0x0000;
// Buffer of characterisitc value.
static uint8_t characteristic1_data[CHARACTERISTIC1_MAX_LEN]={0x01};
static uint8_t characteristic2_data[CHARACTERISTIC2_MAX_LEN]={0x00};
static uint8_t characteristic3_data[CHARACTERISTIC2_MAX_LEN]={0x03};

// Timer task.
static btstack_timer_source_t characteristic2;
// Advertise parameters.
static advParams_t adv_params;
// Advertise data.
static uint8_t adv_data[]={
    0x02,
    BLE_GAP_AD_TYPE_FLAGS,
    BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE,   
    0x11,
    BLE_GAP_AD_TYPE_128BIT_SERVICE_UUID_COMPLETE,
    0x1e, 0x94, 0x8d, 0xf1, 0x48, 0x31, 0x94, 0xba, 0x75, 0x4c, 0x3e, 0x50, 0x00, 0x00, 0x3d, 0x71 
};

static uint8_t scan_response[]={
    0x08,
    BLE_GAP_AD_TYPE_COMPLETE_LOCAL_NAME,
    'R', 'B',  'L', '-', 'D', 'U', 'O'
};
/******************************************************
 *               Function Definitions
 ******************************************************/
/**
 * @brief Connect handle.
 *
 * @param[in]  status   BLE_STATUS_CONNECTION_ERROR or BLE_STATUS_OK.
 * @param[in]  handle   Connect handle.
 *
 * @retval None
 */
void deviceConnectedCallback(BLEStatus_t status, uint16_t handle) {
    switch (status){
        case BLE_STATUS_OK:
            Serial.println("Device connected!");
            break;
        default:
            break;
    }
}

/**
 * @brief Disconnect handle.
 *
 * @param[in]  handle   Connect handle.
 *
 * @retval None
 */
void deviceDisconnectedCallback(uint16_t handle){
    Serial.println("Disconnected.");
}

/**
 * @brief Callback for reading event.
 *
 * @note  If characteristic contains client characteristic configuration,then client characteristic configration handle is value_handle+1.
 *        Now can't add user_descriptor.
 *
 * @param[in]  value_handle    
 * @param[in]  buffer 
 * @param[in]  buffer_size    Ignore it.
 *
 * @retval  Length of current attribute value.
 */
uint16_t gattReadCallback(uint16_t value_handle, uint8_t * buffer, uint16_t buffer_size)
{   
    uint8_t characteristic_len=0;

    Serial.print("Read value handler: ");
    Serial.println(value_handle, HEX);

    if(character1_handle == value_handle)
    {   // Characteristic value handle.
        Serial.println("Character1 read:");
        memcpy(buffer, characteristic1_data, CHARACTERISTIC1_MAX_LEN);
        characteristic_len = CHARACTERISTIC1_MAX_LEN;
    }
    else if(character1_handle+1 == value_handle)
    {   // Client Characteristic Configuration Descriptor Handle.
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

/**
 * @brief Callback for writting event.
 *
 * @param[in]  value_handle  
 * @param[in]  *buffer       The buffer pointer of writting data.
 * @param[in]  size          The length of writting data.   
 *
 * @retval 
 */
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

/**
 * @brief Timer task for sending notify of characteristic to client.
 *
 * @param[in]  *ts   
 *
 * @retval None
 */
static void characteristic2_notify(btstack_timer_source_t *ts)
{
    Serial.println("characteristic2_notify");

    characteristic2_data[CHARACTERISTIC2_MAX_LEN-1]++;
    ble.sendNotify(character2_handle, characteristic2_data, CHARACTERISTIC2_MAX_LEN);
    // Restart timer.
    ble.setTimer(ts, 10000);
    ble.addTimer(ts);
}

/**
 * @brief Setup.
 */
void setup()
{
    Serial.begin(115200);
    delay(5000);
    Serial.println("BLE peripheral demo.");
    
    // Open debugger, must befor init().
    //ble.debugLogger(true);
    //ble.debugError(true);
    //ble.enablePacketLogger();
    
    // Initialize ble_stack.
    ble.init();

    // Register functions.
    ble.onConnectedCallback(deviceConnectedCallback);
    ble.onDisconnectedCallback(deviceDisconnectedCallback);
    ble.onDataReadCallback(gattReadCallback);
    ble.onDataWriteCallback(gattWriteCallback);

    // Add GAP_SERVER  
    ble.addService(0x1800);
    // GATT_UUID_GAP_DEVICE_NAME
    ble.addCharacteristic(0x2A00, ATT_PROPERTY_READ|ATT_PROPERTY_WRITE, (uint8_t*)DEVICE_NAME, sizeof(DEVICE_NAME));
    // GATT_UUID_GAP_ICON
    ble.addCharacteristic(0x2A01, ATT_PROPERTY_READ, appearance, sizeof(appearance));
    // GATT_UUID_GAP_PREF_CONN_PARAM
    ble.addCharacteristic(0x2A04, ATT_PROPERTY_READ, conn_param, sizeof(conn_param));
    // Add GATT_SERVER  
    ble.addService(0x1801);
    // GATT_UUID_GATT_SRV_CHGD
    ble.addCharacteristic(0x2A05, ATT_PROPERTY_INDICATE, change, sizeof(change));

    // Add primary service1.
    ble.addService(service1_uuid);
    // Add characteristic to service1, return value handle of characteristic.
    character1_handle = ble.addCharacteristicDynamic(char1_uuid, ATT_PROPERTY_NOTIFY|ATT_PROPERTY_WRITE_WITHOUT_RESPONSE, characteristic1_data, CHARACTERISTIC1_MAX_LEN);
    character2_handle = ble.addCharacteristicDynamic(char2_uuid, ATT_PROPERTY_READ|ATT_PROPERTY_NOTIFY, characteristic2_data, CHARACTERISTIC2_MAX_LEN);
    // Add primary sevice2.
    ble.addService(service2_uuid);
    character3_handle = ble.addCharacteristic(char3_uuid, ATT_PROPERTY_READ, characteristic3_data, CHARACTERISTIC3_MAX_LEN);
    
    // Set advertise parameters.
    // @note  advertising_interval_min ([0x0020,0x4000], default: 0x0800, unit: 0.625 msec)
    //        advertising_interval_max ([0x0020,0x4000], default: 0x0800, unit: 0.625 msec)
    //        advertising_type (enum from 0: ADV_IND, ADC_DIRECT_IND, ADV_SCAN_IND, ADV_NONCONN_IND)
    //        own_address_type (enum from 0: public device address, random device address)
    //        advertising_channel_map (flags: reserved(0x00),chan_37(0x01), chan_38(0x02), chan_39(0x04), default(0x07,all channels enable))
    // @note  If the advertising_type is set to ADV_SCAN_IND or ADV_NONCONN_IND,advertising_interval_min and advertising_interval_max shal not be set to less than 0x00A0.
    //
    adv_params.adv_int_min = 0x00A0;       
    adv_params.adv_int_max = 0x01A0;
    adv_params.adv_type    = 0;
    adv_params.dir_addr_type = 0;
    memset(adv_params.dir_addr,0,6);
    adv_params.channel_map = 0x07;
    adv_params.filter_policy = 0x00;
    
    ble.setAdvertisementParams(&adv_params);

    ble.setScanResponseData(sizeof(scan_response), scan_response);
    // Set advertise data.
    ble.setAdvertisementData(sizeof(adv_data), adv_data);
    // Start advertising.
    ble.startAdvertising();
    Serial.println("BLE start advertising.");
    
    // set one-shot timer
    characteristic2.process = &characteristic2_notify;
    ble.setTimer(&characteristic2, 10000);
    ble.addTimer(&characteristic2);
}

/**
 * @brief Loop.
 */
void loop()
{
    
}

