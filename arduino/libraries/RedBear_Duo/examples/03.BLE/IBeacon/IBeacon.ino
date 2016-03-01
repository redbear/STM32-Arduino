#if defined(ARDUINO) 
SYSTEM_MODE(MANUAL);//do not connect to cloud
#else
SYSTEM_MODE(AUTOMATIC);//connect to cloud
#endif

static uint8_t  appearance[2]    = {0x00, 0x02};
static uint8_t  change[2]        = {0x00, 0x00};
static uint8_t  conn_param[8]    = {0x28, 0x00, 0x90, 0x01, 0x00, 0x00, 0x90, 0x01};

static advParams_t adv_params;
static uint8_t adv_data[31]={0x02,0x01,0x06, 0x1A,0xFF,0x4C,0x00,0x02,0x15,0x71,0x3d,0x00,0x00,0x50,0x3e,0x4c,0x75,0xba,0x94,0x31,0x48,0xf1,0x8d,0x94,0x1e,0x00,0x00,0x00,0x00,0xC5};


void setup()
{
    Serial.begin(115200);
    delay(5000);
    Serial.println("IBeacon demo.");
    //ble.debugLogger(true);
    ble.init();
    
    adv_params.adv_int_min = 0x00a0;
    adv_params.adv_int_max = 0x0180;
    adv_params.adv_type    = 0;
    adv_params.dir_addr_type = 0;
    memset(adv_params.dir_addr,0,6);
    adv_params.channel_map = 0x07;
    adv_params.filter_policy = 0x00;
    
    ble.setAdvParams(&adv_params);
    
    ble.setAdvData(sizeof(adv_data), adv_data);
    
    ble.startAdvertising();
    Serial.println("BLE start advertising.");
    

}

void loop()
{
    
}

