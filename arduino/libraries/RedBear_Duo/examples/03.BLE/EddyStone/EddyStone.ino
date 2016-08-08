/**
 * This file is part of Faros.
 * 
 * Copyright 2015 Frank Duerr
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 * http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * 
 * Creat by duerrfk
 * 
 * Motify by Jackson_Lv 2016.5
 */

#include <AES.h>
#include"eddystone.h"

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

/* 
 * 10 byte namespace id. Google suggests different methods to create this:
 *     - Truncated hash: first 10 bytes of your SHA1 hash of your FQDN.
 *     - Elided Version 4 UUID: version 4 UUID with bytes 5 - 10 (inclusive) removed 
 */
#define EDDYSTONE_NAMESPACE_ID    0x72, 0x65, 0x64, 0x62, 0x65, 0x61, 0x72, 0x6c, 0x61,0x62
/* 6 byte instance id (any scheme you like). */
#define EDDYSTONE_INSTANCE_ID     0x65, 0x64, 0x64, 0x79, 0x73, 0x74

/* Scheme of the encoded URL. */
const url_schemes eddystone_url_scheme = http_www_dot;
/* 
 * Encoded URL (max. 17 bytes). 
 * The following bytes expand to a sequence of characters:
 *     - 0x00:  ".com/"
 *     - 0x01:  ".org/"
 *     - 0x02:  ".edu/"
 *     - 0x03:  ".net/"
 *     - 0x04:  ".info/"
 *     - 0x05:  ".biz/"
 *     - 0x06:  ".gov/"
 *     - 0x07:  ".com"
 *     - 0x08:  ".org"
 *     - 0x09:  ".edu"
 *     - 0x0a:  ".net"
 *     - 0x0b:  ".info"
 *     - 0x0c:  ".biz"
 *     - 0x0d:  ".gov"
 *     - 0x0e..0x20:  Reserved for Future Use
 *     - 0x7F..0xFF:  Reserved for Future Use
 * E.g., The following example encodes the URL: redbearlab.com. ("http://www." is added by the schema definition)
 */
#define EDDYSTONE_ENCODED_URL     0x72, 0x65, 0x64, 0x62, 0x65, 0x61, 0x72, 0x6c, 0x61,0x62, 0x07

// Eddystone EID
#define EDDYSTONE_EID             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

// Eddystone UID advertising data
static uint8_t uid_adv_data[] = {
  0x02,                                        // Length  Flags. CSS v5, Part A, § 1.3
  BLE_GAP_AD_TYPE_FLAGS,                       // Flags data type value 
  BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE, // Flags data 
    
  0x03,                                        // Length  Complete list of 16-bit Service UUIDs. Ibid. § 1.1
  BLE_GAP_AD_TYPE_16BIT_SERVICE_UUID_COMPLETE, // Complete list of 16-bit Service UUIDs data type value 
  0xAA,0xFE,                                   // 16-bit Eddystone UUID 
  
  0x15,                                        // Length  Service Data. Ibid. § 1.11
  BLE_GAP_AD_TYPE_SERVICE_DATA,                // Service Data data type value  
  0xAA,0xFE,                                   // 16-bit Eddystone UUID 
  EDDYSTONE_FRAME_TYPE_UID,
  (uint8_t)EDDYSTONE_TXPWR,
  EDDYSTONE_NAMESPACE_ID,                      // 10 byte namespace id.
  EDDYSTONE_INSTANCE_ID                        // 6 byte instance id (any scheme you like).
};

// Eddystone URL advertising data
static uint8_t url_adv_data[] = {
  0x02,
  BLE_GAP_AD_TYPE_FLAGS,
  BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE,
  
  0x03,
  BLE_GAP_AD_TYPE_16BIT_SERVICE_UUID_COMPLETE,
  0xAA,0xFE,
  
  0x11,
  BLE_GAP_AD_TYPE_SERVICE_DATA,
  0xAA,0xFE,
  EDDYSTONE_FRAME_TYPE_URL,
  (uint8_t)EDDYSTONE_TXPWR,
  eddystone_url_scheme, EDDYSTONE_ENCODED_URL
};

// Eddystone EID advertising data
static uint8_t eid_adv_data[] = {
  0x02,
  BLE_GAP_AD_TYPE_FLAGS,
  BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE,
  
  0x03,
  BLE_GAP_AD_TYPE_16BIT_SERVICE_UUID_COMPLETE,
  0xAA,0xFE,
  
  0x11,
  BLE_GAP_AD_TYPE_SERVICE_DATA,
  0xAA,0xFE,
  EDDYSTONE_FRAME_TYPE_EID,
  (uint8_t)EDDYSTONE_TXPWR,
  EDDYSTONE_EID
};

//identify key
static uint8_t identify_key[] = "RedBearLab      ";

//use to computing the temporary key
static uint8_t value_temp[16] = { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };

//use to save the temporary key
static const uint8_t const_temp_key[16] = { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0x00 };
static uint8_t temp_key[16] = { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0x00 };

//use to computing the EID value 
static const uint8_t const_EID_value_temp[16] = { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01 };
static uint8_t EID_value_temp[16] = { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01 };

//use to save the EID value 
static uint8_t EID_value[16] = { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };

static uint8_t K = 0;

static uint16_t time_count = 0;
static uint32_t Ktime_count = 1;

#define Button  D1

AES aes;

void sp_time_counter_acheive();
void time_counter_acheive();

// set one-shot timer
Timer t0(1000, sp_time_counter_acheive);
Timer t1(1000, time_counter_acheive);

static uint8_t eddystone_type = 0x10;

// Actual advertising data to be filled in.
static uint8_t adv_data[31] = {
};

/**
 * @brief Connect handle.
 *
 * @param[in]  status   BLE_STATUS_CONNECTION_ERROR or BLE_STATUS_OK.
 * @param[in]  handle   Connect handle.
 *
 * @retval None
 */
void deviceConnectedCallback(BLEStatus_t status, uint16_t handle) {
  switch (status) {
    case BLE_STATUS_OK:
      Serial.println("Device connected!");
      break;
    default: break;
  }
}

/**
 * @brief Disconnect handle.
 *
 * @param[in]  handle   Connect handle.
 *
 * @retval None
 */
void deviceDisconnectedCallback(uint16_t handle) {
  Serial.println("Disconnected.");
}

void sp_time_counter_acheive() {
  if (eddystone_type == EDDYSTONE_FRAME_TYPE_EID) {
    byte succ;
    time_count++;
    temp_key[14] = (0xFF00&time_count) >> 8;
    temp_key[15] = 0xFF & time_count;
    Serial.print("temp_key: ");
    for (uint8_t i = 0; i < sizeof(temp_key); i++) {
      Serial.print(temp_key[i], HEX);
      Serial.print(" ");
    }
    Serial.println("");
    succ = aes.set_key(identify_key, 128);
    succ = aes.encrypt(temp_key, value_temp);
    if (succ != SUCCESS) {
      Serial.println("Failure encrypt");
    }
  }
}

void time_counter_acheive() {
  if (eddystone_type == EDDYSTONE_FRAME_TYPE_EID) {
    byte succ;
    ble.stopAdvertising();
    K++;
    if (K == 16) K = 0;

    Ktime_count = pow(2, K);
    EID_value_temp[11] = K;
    Serial.println("");
    Serial.print("Ktime_count: ");
    Serial.print(Ktime_count);
    Serial.println(" sec");
    EID_value_temp[12] = (0xFF000000 & Ktime_count) >> 24;
    EID_value_temp[13] = (0x00FF0000 & Ktime_count) >> 16;
    EID_value_temp[14] = (0x0000FF00 & Ktime_count) >> 8;
    EID_value_temp[15] = (0x000000FF & Ktime_count);
    succ = aes.set_key(value_temp, 128);
    succ = aes.encrypt(EID_value_temp, EID_value);
    if (succ != SUCCESS) 
      Serial.println("Failure encrypt") ;
    else {
      for (uint8_t i = 0; i < 8; i++) {
        adv_data[i+13] = EID_value[i];
      }
      ble.setAdvertisementData(sizeof(adv_data), adv_data);
      ble.startAdvertising();
    }
    t1.changePeriod(Ktime_count*1000);
    t1.reset();
  }
}

void eddys_type_change() {
  Serial.println("Adv Type Change") ;
  eddystone_type = eddystone_type + 0x10;
  if (eddystone_type == 0x20) {
    eddystone_type = eddystone_type + 0x10;
  }
  else if (eddystone_type == 0x40) {
    eddystone_type = 0;
  }
    
  switch (eddystone_type) {
    case EDDYSTONE_FRAME_TYPE_UID:
      memcpy(adv_data, uid_adv_data, sizeof(uid_adv_data));
      ble.setAdvertisementData(sizeof(adv_data), adv_data);
      ble.startAdvertising();
      break;
    case EDDYSTONE_FRAME_TYPE_URL:
      memcpy(adv_data, url_adv_data, sizeof(url_adv_data));
      ble.setAdvertisementData(sizeof(adv_data), adv_data);
      ble.startAdvertising();
      break;
    case EDDYSTONE_FRAME_TYPE_EID:
      time_count = 0;
      K = 0;
      t1.changePeriod(1000);
      byte succ;
      memcpy(adv_data, eid_adv_data, sizeof(eid_adv_data));
      memcpy(temp_key, const_temp_key, sizeof(const_temp_key));
      memcpy(EID_value_temp, const_EID_value_temp, sizeof(EID_value_temp));
      succ = aes.set_key (identify_key, 128);
      succ = aes.encrypt (temp_key, value_temp);
      if (succ != SUCCESS)
        Serial.println("Failure encrypt");
      else {
        Serial.print("value temp: ");
        for (uint8_t i = 0; i < sizeof(value_temp); i++) {
          Serial.print(value_temp[i],HEX);
          Serial.print(" ");
        }
        Serial.println(" ");
      }
      succ = aes.set_key(value_temp, 128);
      succ = aes.encrypt(EID_value_temp, EID_value);
      if (succ != SUCCESS)
        Serial.println("Failure encrypt");  
      else {
        Serial.print("EID value: ");
        for (uint8_t i = 0; i < sizeof(EID_value); i++) {
          Serial.print(EID_value[i], HEX);
          Serial.print(" ");
        }
        Serial.println(" ");
    
        for (uint8_t i = 0; i < 8; i++) {
          adv_data[i+13] = EID_value[i];
        }  
      }
          
      ble.setAdvertisementData(sizeof(adv_data), adv_data);     
      ble.startAdvertising();

      if(t0.isActive()) {
        t0.reset();
        t1.reset();
      }
      else {
        t0.start();
        t1.start();
      }
      break;
    default: break;
  }
}

void setup() {
  byte succ;
  Serial.begin(115200);
  delay(5000);
  Serial.println("EddyStone demo.");
  
  //ble.debugLogger(true);
  // Initialize ble_stack.
  ble.init();

  // Register BLE callback functions
  ble.onConnectedCallback(deviceConnectedCallback);
  ble.onDisconnectedCallback(deviceDisconnectedCallback);

  ble.setAdvertisementParams(&adv_params);
    
  switch(eddystone_type) {
    case EDDYSTONE_FRAME_TYPE_UID:
      memcpy(adv_data, uid_adv_data, sizeof(uid_adv_data));
      ble.setAdvertisementData(sizeof(adv_data), adv_data);
      ble.startAdvertising();
      Serial.println("startAdvertising ");
      break;
    case EDDYSTONE_FRAME_TYPE_URL:
      memcpy(adv_data, url_adv_data, sizeof(url_adv_data));
      ble.setAdvertisementData(sizeof(adv_data), adv_data);
      ble.startAdvertising();
      Serial.println("startAdvertising ");
      break;
    case EDDYSTONE_FRAME_TYPE_EID:
      byte succ;
      memcpy(adv_data, eid_adv_data, sizeof(eid_adv_data));
      memcpy(temp_key, const_temp_key, sizeof(const_temp_key));
      memcpy(EID_value_temp, const_EID_value_temp, sizeof(EID_value_temp));
      succ = aes.set_key(identify_key, 128);
      succ = aes.encrypt(temp_key, value_temp);
      if (succ != SUCCESS)
        Serial.println("Failure encrypt");
      else {
        Serial.print("value temp: ");
        for (uint8_t i = 0; i < sizeof(value_temp); i++) {
          Serial.print(value_temp[i],HEX);
          Serial.print(" ");
        }
        Serial.println(" ");
      }
      succ = aes.set_key(value_temp, 128);
      succ = aes.encrypt(EID_value_temp, EID_value);
      if (succ != SUCCESS)
        Serial.println("Failure encrypt");  
      else {
        Serial.print("EID value: ");
        for (uint8_t i = 0; i < sizeof(EID_value); i++) {
          Serial.print(EID_value[i],HEX);
          Serial.print(" ");
        }
        Serial.println(" ");
    
        for (uint8_t i = 0; i < 8; i++) {
          adv_data[i+13] = EID_value[i];
        }  
      }
          
      ble.setAdvertisementData(sizeof(adv_data), adv_data);     
      ble.startAdvertising();
      Serial.println("startAdvertising ");

      if (t0.isActive()) {
        t0.reset();
        t1.reset();
      }
      else {
        t0.start();
        t1.start();
      }
      break;
    default: break;
  }
  pinMode(Button, INPUT_PULLDOWN);
  attachInterrupt(D1, eddys_type_change, RISING);
}

void loop() {
    
}

