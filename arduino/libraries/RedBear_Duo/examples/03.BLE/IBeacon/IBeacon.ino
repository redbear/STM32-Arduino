/*
 * Copyright (c) 2016 RedBear
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), 
 * to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS 
 * IN THE SOFTWARE.
 */
 
/*
 * SYSTEM_MODE:
 *     - AUTOMATIC: Automatically try to connect to Wi-Fi and the Particle Cloud and handle the cloud messages.
 *     - SEMI_AUTOMATIC: Manually connect to Wi-Fi and the Particle Cloud, but automatically handle the cloud messages.
 *     - MANUAL: Manually connect to Wi-Fi and the Particle Cloud and handle the cloud messages.
 *     
 * SYSTEM_MODE(AUTOMATIC) does not need to be called, because it is the default state. 
 * However the user can invoke this method to make the mode explicit.
 * Learn more about system modes: https://docs.particle.io/reference/firmware/photon/#system-modes .
 */
#if defined(ARDUINO) 
SYSTEM_MODE(SEMI_AUTOMATIC); 
#endif

/* 
 * BLE peripheral advertising parameters:
 *     - advertising_interval_min: [0x0020, 0x4000], default: 0x0800, unit: 0.625 msec
 *     - advertising_interval_max: [0x0020, 0x4000], default: 0x0800, unit: 0.625 msec
 *     - advertising_type: 
 *           BLE_GAP_ADV_TYPE_ADV_IND 
 *           BLE_GAP_ADV_TYPE_ADV_DIRECT_IND 
 *           BLE_GAP_ADV_TYPE_ADV_SCAN_IND 
 *           BLE_GAP_ADV_TYPE_ADV_NONCONN_IND
 *     - own_address_type: 
 *           BLE_GAP_ADDR_TYPE_PUBLIC 
 *           BLE_GAP_ADDR_TYPE_RANDOM
 *     - advertising_channel_map: 
 *           BLE_GAP_ADV_CHANNEL_MAP_37 
 *           BLE_GAP_ADV_CHANNEL_MAP_38 
 *           BLE_GAP_ADV_CHANNEL_MAP_39 
 *           BLE_GAP_ADV_CHANNEL_MAP_ALL
 *     - filter policies: 
 *           BLE_GAP_ADV_FP_ANY 
 *           BLE_GAP_ADV_FP_FILTER_SCANREQ 
 *           BLE_GAP_ADV_FP_FILTER_CONNREQ 
 *           BLE_GAP_ADV_FP_FILTER_BOTH
 *     
 * Note:  If the advertising_type is set to BLE_GAP_ADV_TYPE_ADV_SCAN_IND or BLE_GAP_ADV_TYPE_ADV_NONCONN_IND, 
 *        the advertising_interval_min and advertising_interval_max should not be set to less than 0x00A0.
 */
static advParams_t adv_params = {
  .adv_int_min   = 0x00A0,
  .adv_int_max   = 0x01A0,
  .adv_type      = BLE_GAP_ADV_TYPE_ADV_NONCONN_IND,
  .dir_addr_type = BLE_GAP_ADDR_TYPE_PUBLIC,
  .dir_addr      = {0,0,0,0,0,0},
  .channel_map   = BLE_GAP_ADV_CHANNEL_MAP_ALL,
  .filter_policy = BLE_GAP_ADV_FP_ANY
};

// iBeacon advertising data
static uint8_t adv_data[31] = {
  0x02,
  BLE_GAP_AD_TYPE_FLAGS,
  BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE, 
    
  0x1A,
  BLE_GAP_AD_TYPE_MANUFACTURER_SPECIFIC_DATA,
  0x4C,0x00,    // Company Identifier Code = Apple
  0x02,         // Type = iBeacon
  0x15,         // Following data length
  0x71,0x3d,0x00,0x00,0x50,0x3e,0x4c,0x75,0xba,0x94,0x31,0x48,0xf1,0x8d,0x94,0x1e, // Beacon UUID
  0x00,0x00,    // Major
  0x00,0x00,    // Minor
  0xC5          // Measure Power
};

void setup() {
  Serial.begin(115200);
  delay(5000);
  Serial.println("IBeacon demo.");
  
  //ble.debugLogger(true);
  ble.init();
    
  ble.setAdvertisementParams(&adv_params);
  ble.setAdvertisementData(sizeof(adv_data), adv_data);
    
  ble.startAdvertising();
  Serial.println("BLE start advertising.");
}

void loop() {
    
}

