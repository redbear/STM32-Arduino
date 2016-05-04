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
 * 
 */
 
#include"eddystone.h"
#if defined(ARDUINO) 
SYSTEM_MODE(MANUAL);//do not connect to cloud
#else
SYSTEM_MODE(AUTOMATIC);//connect to cloud
#endif

#define EDDSTONEURL

static advParams_t adv_params;

// 10 byte namespace id. Google suggests different methods to create this:
// - Truncated hash: first 10 bytes of your SHA1 hash of your FQDN.
// - Elided Version 4 UUID: version 4 UUID with bytes 5 - 10 (inclusive) removed 
const uint8_t eddystone_namespace_id[10] = {0x4F, 0xFB, 0x0B, 0x0A, 0xA9, 0x21, 0x0F, 0xE6, 0xD1, 0xD2};

// 6 byte instance id (any scheme you like).
const uint8_t eddystone_instance_id[6] = {0x4A, 0xB5, 0x16, 0xDC, 0xBD, 0xC2};

// Scheme of the encoded URL.
const url_schemes eddystone_url_scheme = http_www_dot;
// Encoded URL (max. 17 bytes)
// The following bytes expand to a sequence of characters:
// 0x00  .com/
// 0x01  .org/
// 0x02  .edu/
// 0x03  .net/
// 0x04  .info/
// 0x05  .biz/
// 0x06  .gov/
// 0x07  .com
// 0x08  .org
// 0x09  .edu
// 0x0a  .net
// 0x0b  .info
// 0x0c  .biz
// 0x0d  .gov
// 14..32  0x0e..0x20  Reserved for Future Use
// 127..255  0x7F..0xFF  Reserved for Future Use
// The following example encodes the URL frank-duerr.de
// ("http://www." is added by the schema definition)
const uint8_t eddystone_enc_url[] = {0x72, 0x65, 0x64, 0x62, 0x65, 0x61, 0x72, 0x6c, 0x61,0x62, 0x07};//redbear.com

// TLM version
const uint8_t eddystone_tlm_version = 0x00;

// 10 byte namespace id. Google suggests different methods to create this:
// - Truncated hash: first 10 bytes of your SHA1 hash of your FQDN.
// - Elided Version 4 UUID: version 4 UUID with bytes 5 - 10 (inclusive) removed 
// 6 byte instance id (any scheme you like).
#if defined EDDSTONEUID
static uint8_t adv_data[]={
    EDDYSTONE_FRAME_TYPE_UID,
    (uint8_t)EDDYSTONE_TXPWR,
    0x4F, 0xFB, 0x0B, 0x0A, 0xA9, 0x21, 0x0F, 0xE6, 0xD1, 0xD2,
    0x4A, 0xB5, 0x16, 0xDC, 0xBD, 0xC2
};
#elif defined EDDSTONEURL
static uint8_t adv_data[]={
    EDDYSTONE_FRAME_TYPE_URL,
    (uint8_t)EDDYSTONE_TXPWR,
    http_www_dot, 0x72, 0x65, 0x64, 0x62, 0x65, 0x61, 0x72, 0x6c, 0x61,0x62, 0x07//redbear.com
};
#endif

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

