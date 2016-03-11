/******************************************************
 *                      Macros
 ******************************************************/
#if defined(ARDUINO) 
SYSTEM_MODE(MANUAL);//do not connect to cloud
#else
SYSTEM_MODE(AUTOMATIC);//connect to cloud
#endif

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

static uint8_t  appearance[2]    = {0x00, 0x02};
static uint8_t  change[2]        = {0x00, 0x00};
static uint8_t  conn_param[8]    = {0x28, 0x00, 0x90, 0x01, 0x00, 0x00, 0x90, 0x01};

static uint16_t character1_handle = 0x0000;
static uint16_t character2_handle = 0x0000;
static uint16_t character3_handle = 0x0000;

static uint8_t characteristic1_data[CHARACTERISTIC1_MAX_LEN]={0x01};
static uint8_t characteristic2_data[CHARACTERISTIC2_MAX_LEN]={0x00};


static btstack_timer_source_t characteristic2;

static advParams_t adv_params;
static uint8_t adv_data[]={0x02,0x01,0x06,0x08,0x08,'B','i','s','c','u','i','t',0x11,0x07,0x1e,0x94,0x8d,0xf1,0x48,0x31,0x94,0xba,0x75,0x4c,0x3e,0x50,0x00,0x00,0x3d,0x71};

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

    if(character1_handle == value_handle)
    {
        memcpy(characteristic1_data, buffer, min(size,CHARACTERISTIC1_MAX_LEN));
        Serial.print("Characteristic1 write value: ");
        for(uint8_t index=0; index<min(size,CHARACTERISTIC1_MAX_LEN); index++)
        {
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
    if(rx_state != 0)
    {
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
    
    adv_params.adv_int_min = 0x00A0;
    adv_params.adv_int_max = 0x01A0;
    adv_params.adv_type    = 0;
    adv_params.dir_addr_type = 0;
    memset(adv_params.dir_addr,0,6);
    adv_params.channel_map = 0x07;
    adv_params.filter_policy = 0x00;
    
    ble.setAdvParams(&adv_params);
    
    ble.setAdvData(sizeof(adv_data), adv_data);

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

