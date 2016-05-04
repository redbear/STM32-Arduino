
#if defined(ARDUINO) 
SYSTEM_MODE(MANUAL);//do not connect to cloud
#else
SYSTEM_MODE(AUTOMATIC);//connect to cloud
#endif

static advParams_t adv_params;

/******************** IBeacon Format ***************************/                  
/* 02 01 06 1A FF 4C 00 02 15: iBeacon prefix (fixed)
  71 3d 00 00 50 3e 4c 75 ba 94 31 48 f1 8d 94 1e: proximity UUID
  00 49: major
  00 0A: minor
  C5: 2’s complement of measured TX power*/
  
static uint8_t adv_data[31]={
    0x02,
    BLE_GAP_AD_TYPE_FLAGS,
    BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE, 
    0x1A,
    BLE_GAP_AD_TYPE_MANUFACTURER_SPECIFIC_DATA,
    0x4C,0x00,0x02,0x15,0x71,0x3d,0x00,0x00,0x50,0x3e,0x4c,0x75,0xba,0x94,0x31,0x48,0xf1,0x8d,0x94,0x1e,0x00,0x00,0x00,0x00,0xC5
};


void setup()
{
    Serial.begin(115200);
    delay(5000);
    Serial.println("IBeacon demo.");
    //ble.debugLogger(true);
    ble.init();
    
    adv_params.adv_int_min = 0x00A0;
    adv_params.adv_int_max = 0x01A0;
    adv_params.adv_type    = 3;
    adv_params.dir_addr_type = 0;
    memset(adv_params.dir_addr,0,6);
    adv_params.channel_map = 0x07;
    adv_params.filter_policy = 0x00;
    
    ble.setAdvertisementParams(&adv_params);
    
    ble.setAdvertisementData(sizeof(adv_data), adv_data);
    
    ble.startAdvertising();
    Serial.println("BLE start advertising.");
}

void loop()
{
    
}

