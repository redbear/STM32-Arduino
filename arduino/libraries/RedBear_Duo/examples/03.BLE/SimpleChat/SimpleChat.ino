/******************************************************
 *Download RedBear "BLE Controller" APP 
 *From APP store(IOS)/Play Store(Android)                                         
 ******************************************************/
/******************************************************
 *                      Macros
 ******************************************************/
#if defined(ARDUINO) 
SYSTEM_MODE(MANUAL);//do not connect to cloud
#else
SYSTEM_MODE(AUTOMATIC);//connect to cloud
#endif

// BLE peripheral preferred connection parameters
#define MIN_CONN_INTERVAL          0x0028 // 50ms. Minimum connection interval = MIN_CONN_INTERVAL * 1.25 ms, where MIN_CONN_INTERVAL ranges from 0x0006 to 0x0C80.
#define MAX_CONN_INTERVAL          0x0190 // 500ms. Maximum connection interval = MAX_CONN_INTERVAL * 1.25 ms,  where MAX_CONN_INTERVAL ranges from 0x0006 to 0x0C80.
#define SLAVE_LATENCY              0x0000 // No slave latency. The SLAVE_LATENCY ranges from 0x0000 to 0x03E8.
#define CONN_SUPERVISION_TIMEOUT   0x03E8 // 10s. Connection supervision timeout = CONN_SUPERVISION_TIMEOUT * 10 ms, where CONN_SUPERVISION_TIMEOUT ranges from 0x000A to 0x0C80.

// Learn about appearance: http://developer.bluetooth.org/gatt/characteristics/Pages/CharacteristicViewer.aspx?u=org.bluetooth.characteristic.gap.appearance.xml
#define BLE_PERIPHERAL_APPEARANCE  BLE_APPEARANCE_UNKNOWN

#define DEVICE_NAME                "Simple Chat"

#define CHARACTERISTIC1_MAX_LEN    15
#define CHARACTERISTIC2_MAX_LEN    15
#define TXRX_BUF_LEN               15

/******************************************************
 *               Variable Definitions
 ******************************************************/
static uint8_t service1_uuid[16]       ={0x71,0x3d,0x00,0x00,0x50,0x3e,0x4c,0x75,0xba,0x94,0x31,0x48,0xf1,0x8d,0x94,0x1e};
static uint8_t service1_tx_uuid[16]    ={0x71,0x3d,0x00,0x03,0x50,0x3e,0x4c,0x75,0xba,0x94,0x31,0x48,0xf1,0x8d,0x94,0x1e};
static uint8_t service1_rx_uuid[16]    ={0x71,0x3d,0x00,0x02,0x50,0x3e,0x4c,0x75,0xba,0x94,0x31,0x48,0xf1,0x8d,0x94,0x1e};

static uint8_t  appearance[2] = { 
    LOW_BYTE(BLE_PERIPHERAL_APPEARANCE), 
    HIGH_BYTE(BLE_PERIPHERAL_APPEARANCE) 
};

static uint8_t  change[4] = {
    0x00, 0x00, 0xFF, 0xFF
};
// BLE connection params
// Connection interval 
//    Range: 0x0006 to 0x0C80
//    Time = N * 1.25 msec
//    Time Range: 7.5 msec to 4000 msec.
//Slave latency 
//    Range: 0x0000 to 0x01F3
//Connection supervision timeout 
//    Range: 0x000A to 0x0C80
//    Time = N * 10 msec
//    Time Range: 100 msec to 32 seconds
static uint8_t  conn_param[8] = {
    LOW_BYTE(MIN_CONN_INTERVAL), HIGH_BYTE(MIN_CONN_INTERVAL), 
    LOW_BYTE(MAX_CONN_INTERVAL), HIGH_BYTE(MAX_CONN_INTERVAL), 
    LOW_BYTE(SLAVE_LATENCY), HIGH_BYTE(SLAVE_LATENCY), 
    LOW_BYTE(CONN_SUPERVISION_TIMEOUT), HIGH_BYTE(CONN_SUPERVISION_TIMEOUT)
};

// BLE peripheral advertising parameters
// Note  advertising_interval_min ([0x0020,0x4000], default: 0x0800, unit: 0.625 msec)
//        advertising_interval_max ([0x0020,0x4000], default: 0x0800, unit: 0.625 msec)
//        advertising_type (enum from 0: BLE_GAP_ADV_TYPE_ADV_IND, BLE_GAP_ADV_TYPE_ADV_DIRECT_IND, BLE_GAP_ADV_TYPE_ADV_SCAN_IND, BLE_GAP_ADV_TYPE_ADV_NONCONN_IND)
//        own_address_type (enum from 0: BLE_GAP_ADDR_TYPE_PUBLIC, BLE_GAP_ADDR_TYPE_RANDOM)
//        advertising_channel_map (flags: BLE_GAP_ADV_CHANNEL_MAP_37, BLE_GAP_ADV_CHANNEL_MAP_38, BLE_GAP_ADV_CHANNEL_MAP_39, BLE_GAP_ADV_CHANNEL_MAP_ALL)
//        filter policies (enum from 0: BLE_GAP_ADV_FP_ANY, BLE_GAP_ADV_FP_FILTER_SCANREQ, BLE_GAP_ADV_FP_FILTER_CONNREQ, BLE_GAP_ADV_FP_FILTER_BOTH)
// Note  If the advertising_type is set to BLE_GAP_ADV_TYPE_ADV_SCAN_IND or BLE_GAP_ADV_TYPE_ADV_NONCONN_IND,advertising_interval_min and advertising_interval_max shal not be set to less than 0x00A0.
static advParams_t adv_params = {
    .adv_int_min   = 0x0030,
    .adv_int_max   = 0x0030,
    .adv_type      = BLE_GAP_ADV_TYPE_ADV_IND,
    .dir_addr_type = BLE_GAP_ADDR_TYPE_PUBLIC,
    .dir_addr      = {0,0,0,0,0,0},
    .channel_map   = BLE_GAP_ADV_CHANNEL_MAP_ALL,
    .filter_policy = BLE_GAP_ADV_FP_ANY
};

static uint16_t character1_handle = 0x0000;
static uint16_t character2_handle = 0x0000;
static uint16_t character3_handle = 0x0000;

static uint8_t characteristic1_data[CHARACTERISTIC1_MAX_LEN]={0x01};
static uint8_t characteristic2_data[CHARACTERISTIC2_MAX_LEN]={0x00};

static uint8_t adv_data[]={
    0x02,
    BLE_GAP_AD_TYPE_FLAGS,
    BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE,
    0x08,
    BLE_GAP_AD_TYPE_SHORT_LOCAL_NAME,
    'B','i','s','c','u','i','t',
    0x11,
    BLE_GAP_AD_TYPE_128BIT_SERVICE_UUID_COMPLETE,
    0x1e,0x94,0x8d,0xf1,0x48,0x31,0x94,0xba,0x75,0x4c,0x3e,0x50,0x00,0x00,0x3d,0x71
};

static btstack_timer_source_t characteristic2;

char rx_buf[TXRX_BUF_LEN];
static uint8_t rx_buf_num;
static uint8_t rx_state=0;

/******************************************************
 *               Function Definitions
 ******************************************************/
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

int gattWriteCallback(uint16_t value_handle, uint8_t *buffer, uint16_t size)
{
    Serial.print("Write value handler: ");
    Serial.println(value_handle, HEX);

    if(character1_handle == value_handle) {
        memcpy(characteristic1_data, buffer, min(size,CHARACTERISTIC1_MAX_LEN));
        Serial.print("Characteristic1 write value: ");
        for(uint8_t index=0; index<min(size,CHARACTERISTIC1_MAX_LEN); index++) {
            Serial.print(characteristic1_data[index], HEX);
            Serial.print(" ");
        }
        Serial.println(" ");
    }
    return 0;
}

/*void m_uart_rx_handle()
{   //update characteristic data
    ble.sendNotify(character2_handle, rx_buf, CHARACTERISTIC2_MAX_LEN);
    memset(rx_buf, 0x00,20);
    rx_state = 0;
}*/

static void  characteristic2_notify(btstack_timer_source_t *ts)
{   
    if (Serial.available()) {
      //read the serial command into a buffer
      uint8_t rx_len =min(Serial.available(),CHARACTERISTIC2_MAX_LEN);
      Serial.readBytes(rx_buf, rx_len);
      //send the serial command to the server
      Serial.print("Sent: ");
      Serial.println(rx_buf);
      rx_state = 1;
    }
    if(rx_state != 0) {
        ble.sendNotify(character2_handle, (uint8_t*)rx_buf, CHARACTERISTIC2_MAX_LEN);
        memset(rx_buf, 0x00,20);
        rx_state = 0;
    }
    // reset
    ble.setTimer(ts, 200);
    ble.addTimer(ts);
}

void setup()
{
    Serial.begin(115200);
    delay(5000);
    Serial.println("Simple Chat demo.");
    
    //ble.debugLogger(true);
    ble.init();

    ble.onConnectedCallback(deviceConnectedCallback);
    ble.onDisconnectedCallback(deviceDisconnectedCallback);
    ble.onDataWriteCallback(gattWriteCallback);

    ble.addService(0x1800);
    ble.addCharacteristic(0x2A00, ATT_PROPERTY_READ|ATT_PROPERTY_WRITE, (uint8_t*)DEVICE_NAME, sizeof(DEVICE_NAME));
    ble.addCharacteristic(0x2A01, ATT_PROPERTY_READ, appearance, sizeof(appearance));
    ble.addCharacteristic(0x2A04, ATT_PROPERTY_READ, conn_param, sizeof(conn_param));
    ble.addService(0x1801);
    ble.addCharacteristic(0x2A05, ATT_PROPERTY_INDICATE, change, sizeof(change));

    ble.addService(service1_uuid);
    character1_handle = ble.addCharacteristicDynamic(service1_tx_uuid, ATT_PROPERTY_NOTIFY|ATT_PROPERTY_WRITE|ATT_PROPERTY_WRITE_WITHOUT_RESPONSE, characteristic1_data, CHARACTERISTIC1_MAX_LEN);
    character2_handle = ble.addCharacteristicDynamic(service1_rx_uuid, ATT_PROPERTY_NOTIFY, characteristic2_data, CHARACTERISTIC2_MAX_LEN);
    
    ble.setAdvertisementParams(&adv_params);
    
    ble.setAdvertisementData(sizeof(adv_data), adv_data);

    ble.startAdvertising();

    // set one-shot timer
    characteristic2.process = &characteristic2_notify;
    ble.setTimer(&characteristic2, 500);//100ms
    ble.addTimer(&characteristic2);
    
    Serial.println("BLE start advertising.");
}

void loop()
{
    
}

