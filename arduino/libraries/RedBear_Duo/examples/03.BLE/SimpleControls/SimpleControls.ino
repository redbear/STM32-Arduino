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
 * Download RedBear "BLE Controller" APP from APP store(IOS)/Play Store(Android) to play with this sketch
 */
 
/******************************************************
 *                      Macros
 ******************************************************/
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
 * BLE peripheral preferred connection parameters:
 *     - Minimum connection interval = MIN_CONN_INTERVAL * 1.25 ms, where MIN_CONN_INTERVAL ranges from 0x0006 to 0x0C80
 *     - Maximum connection interval = MAX_CONN_INTERVAL * 1.25 ms,  where MAX_CONN_INTERVAL ranges from 0x0006 to 0x0C80
 *     - The SLAVE_LATENCY ranges from 0x0000 to 0x03E8
 *     - Connection supervision timeout = CONN_SUPERVISION_TIMEOUT * 10 ms, where CONN_SUPERVISION_TIMEOUT ranges from 0x000A to 0x0C80
 */
#define MIN_CONN_INTERVAL          0x0028 // 50ms. 
#define MAX_CONN_INTERVAL          0x0190 // 500ms. 
#define SLAVE_LATENCY              0x0000 // No slave latency. 
#define CONN_SUPERVISION_TIMEOUT   0x03E8 // 10s. 

// Learn about appearance: http://developer.bluetooth.org/gatt/characteristics/Pages/CharacteristicViewer.aspx?u=org.bluetooth.characteristic.gap.appearance.xml
#define BLE_PERIPHERAL_APPEARANCE  BLE_APPEARANCE_UNKNOWN

#define BLE_DEVICE_NAME            "Simple Controls"

#define CHARACTERISTIC1_MAX_LEN    3
#define CHARACTERISTIC2_MAX_LEN    3

#define DIGITAL_OUT_PIN            D2
#define DIGITAL_IN_PIN             A4
#define PWM_PIN                    D3
#define SERVO_PIN                  D4
#define ANALOG_IN_PIN              A5

Servo myservo;

/******************************************************
 *               Variable Definitions
 ******************************************************/
static uint8_t service1_uuid[16]    = { 0x71,0x3d,0x00,0x00,0x50,0x3e,0x4c,0x75,0xba,0x94,0x31,0x48,0xf1,0x8d,0x94,0x1e };
static uint8_t service1_tx_uuid[16] = { 0x71,0x3d,0x00,0x03,0x50,0x3e,0x4c,0x75,0xba,0x94,0x31,0x48,0xf1,0x8d,0x94,0x1e };
static uint8_t service1_rx_uuid[16] = { 0x71,0x3d,0x00,0x02,0x50,0x3e,0x4c,0x75,0xba,0x94,0x31,0x48,0xf1,0x8d,0x94,0x1e };

static uint8_t  appearance[2] = { 
  LOW_BYTE(BLE_PERIPHERAL_APPEARANCE), 
  HIGH_BYTE(BLE_PERIPHERAL_APPEARANCE) 
};

static uint8_t  change[4] = {
  0x00, 0x00, 0xFF, 0xFF
};

static uint8_t  conn_param[8] = {
  LOW_BYTE(MIN_CONN_INTERVAL), HIGH_BYTE(MIN_CONN_INTERVAL), 
  LOW_BYTE(MAX_CONN_INTERVAL), HIGH_BYTE(MAX_CONN_INTERVAL), 
  LOW_BYTE(SLAVE_LATENCY), HIGH_BYTE(SLAVE_LATENCY), 
  LOW_BYTE(CONN_SUPERVISION_TIMEOUT), HIGH_BYTE(CONN_SUPERVISION_TIMEOUT)
};

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
  .adv_int_min   = 0x0030,
  .adv_int_max   = 0x0030,
  .adv_type      = BLE_GAP_ADV_TYPE_ADV_IND,
  .dir_addr_type = BLE_GAP_ADDR_TYPE_PUBLIC,
  .dir_addr      = {0,0,0,0,0,0},
  .channel_map   = BLE_GAP_ADV_CHANNEL_MAP_ALL,
  .filter_policy = BLE_GAP_ADV_FP_ANY
};

static uint8_t adv_data[] = {
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

static uint16_t character1_handle = 0x0000;
static uint16_t character2_handle = 0x0000;
static uint16_t character3_handle = 0x0000;

static uint8_t characteristic1_data[CHARACTERISTIC1_MAX_LEN] = { 0x01 };
static uint8_t characteristic2_data[CHARACTERISTIC2_MAX_LEN] = { 0x00 };

static btstack_timer_source_t characteristic2;

// Mark whether need to notify analog value to client.
static boolean analog_enabled = false;

// Input pin state.
static byte old_state = LOW;

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

/**
 * @brief Callback for writting event.
 *
 * @param[in]  value_handle  
 * @param[in]  *buffer       The buffer pointer of writting data.
 * @param[in]  size          The length of writting data.   
 *
 * @retval 
 */
int gattWriteCallback(uint16_t value_handle, uint8_t *buffer, uint16_t size) {
  Serial.print("Write value handler: ");
  Serial.println(value_handle, HEX);

  if (character1_handle == value_handle) {
    memcpy(characteristic1_data, buffer, CHARACTERISTIC1_MAX_LEN);
    Serial.print("Characteristic1 write value: ");
    for (uint8_t index = 0; index < CHARACTERISTIC1_MAX_LEN; index++) {
      Serial.print(characteristic1_data[index], HEX);
      Serial.print(" ");
    }
    Serial.println(" ");
    //Process the data
    if (characteristic1_data[0] == 0x01) { // Command is to control digital out pin
      if (characteristic1_data[1] == 0x01)
        digitalWrite(DIGITAL_OUT_PIN, HIGH);
      else
        digitalWrite(DIGITAL_OUT_PIN, LOW);
    }
    else if (characteristic1_data[0] == 0xA0) { // Command is to enable analog in reading
      if (characteristic1_data[1] == 0x01)
        analog_enabled = true;
      else
        analog_enabled = false;
    }
    else if (characteristic1_data[0] == 0x02) { // Command is to control PWM pin
      analogWrite(PWM_PIN, characteristic1_data[1]);
    }
    else if (characteristic1_data[0] == 0x03) { // Command is to control Servo pin
      myservo.write(characteristic1_data[1]);
    }
    else if (characteristic1_data[0] == 0x04) { // Command is to initialize all.
      analog_enabled = false;
      myservo.write(0);
      analogWrite(PWM_PIN, 0);
      digitalWrite(DIGITAL_OUT_PIN, LOW);
      old_state = LOW;
    }
  }
  return 0;
}

/**
 * @brief Timer task for sending status change to client.
 *
 * @param[in]  *ts   
 *
 * @retval None
 */
static void  characteristic2_notify(btstack_timer_source_t *ts) {
  if (analog_enabled) { // if analog reading enabled.
    //Serial.println("characteristic2_notify analog reading ");
    // Read and send out
    uint16_t value = analogRead(ANALOG_IN_PIN);
    characteristic2_data[0] = (0x0B);
    characteristic2_data[1] = (value >> 8);
    characteristic2_data[2] = (value);
    if (ble.attServerCanSendPacket())
      ble.sendNotify(character2_handle, characteristic2_data, CHARACTERISTIC2_MAX_LEN);
  }
  // If digital in changes, report the state.
  if (digitalRead(DIGITAL_IN_PIN) != old_state) {
    Serial.println("characteristic2_notify digital reading ");
    old_state = digitalRead(DIGITAL_IN_PIN);
    if (digitalRead(DIGITAL_IN_PIN) == HIGH) {
      characteristic2_data[0] = (0x0A);
      characteristic2_data[1] = (0x01);
      characteristic2_data[2] = (0x00);
      ble.sendNotify(character2_handle, characteristic2_data, CHARACTERISTIC2_MAX_LEN);
    }
    else {
      characteristic2_data[0] = (0x0A);
      characteristic2_data[1] = (0x00);
      characteristic2_data[2] = (0x00);
      ble.sendNotify(character2_handle, characteristic2_data, CHARACTERISTIC2_MAX_LEN);
    }
  }
  // Restart timer.
  ble.setTimer(ts, 200);
  ble.addTimer(ts);
}

/**
 * @brief Setup.
 */
void setup() {
  Serial.begin(115200);
  delay(5000);
  Serial.println("Simple Controls demo.");
  
  //ble.debugLogger(true);
  // Initialize ble_stack.
  ble.init();

  // Register BLE callback functions
  ble.onConnectedCallback(deviceConnectedCallback);
  ble.onDisconnectedCallback(deviceDisconnectedCallback);
  ble.onDataWriteCallback(gattWriteCallback);

  // Add GAP service and characteristics
  ble.addService(BLE_UUID_GAP);
  ble.addCharacteristic(BLE_UUID_GAP_CHARACTERISTIC_DEVICE_NAME, ATT_PROPERTY_READ|ATT_PROPERTY_WRITE, (uint8_t*)BLE_DEVICE_NAME, sizeof(BLE_DEVICE_NAME));
  ble.addCharacteristic(BLE_UUID_GAP_CHARACTERISTIC_APPEARANCE, ATT_PROPERTY_READ, appearance, sizeof(appearance));
  ble.addCharacteristic(BLE_UUID_GAP_CHARACTERISTIC_PPCP, ATT_PROPERTY_READ, conn_param, sizeof(conn_param));

  // Add GATT service and characteristics
  ble.addService(BLE_UUID_GATT);
  ble.addCharacteristic(BLE_UUID_GATT_CHARACTERISTIC_SERVICE_CHANGED, ATT_PROPERTY_INDICATE, change, sizeof(change));

  // Add user defined service and characteristics
  ble.addService(service1_uuid);
  character1_handle = ble.addCharacteristicDynamic(service1_tx_uuid, ATT_PROPERTY_NOTIFY|ATT_PROPERTY_WRITE|ATT_PROPERTY_WRITE_WITHOUT_RESPONSE, characteristic1_data, CHARACTERISTIC1_MAX_LEN);
  character2_handle = ble.addCharacteristicDynamic(service1_rx_uuid, ATT_PROPERTY_NOTIFY, characteristic2_data, CHARACTERISTIC2_MAX_LEN);

  // Set BLE advertising parameters
  ble.setAdvertisementParams(&adv_params);

  // Set BLE advertising data
  ble.setAdvertisementData(sizeof(adv_data), adv_data);

  // BLE peripheral starts advertising now.
  ble.startAdvertising();
  Serial.println("BLE start advertising.");

  // Initialize all peripheral.
  pinMode(DIGITAL_OUT_PIN, OUTPUT);
  pinMode(DIGITAL_IN_PIN, INPUT_PULLUP);
  pinMode(PWM_PIN, OUTPUT);

  // Default to internally pull high, change it if you need
  digitalWrite(DIGITAL_IN_PIN, HIGH);

  myservo.attach(SERVO_PIN);
  myservo.write(0);

  // Start a task to check status.
  characteristic2.process = &characteristic2_notify;
  ble.setTimer(&characteristic2, 500);//2000ms
  ble.addTimer(&characteristic2);
}

/**
 * @brief Loop.
 */
void loop() {
    
}
