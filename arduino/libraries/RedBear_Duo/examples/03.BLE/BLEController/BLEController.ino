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
 
#include "pin_inf.h"

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

#define BLE_DEVICE_NAME            "BLE Controller"

#define TXRX_BUF_LEN               8  

#define PIN_CAPABILITY_NONE        0x00
#define PIN_CAPABILITY_DIGITAL     0x01
#define PIN_CAPABILITY_ANALOG      0x02
#define PIN_CAPABILITY_PWM         0x04
#define PIN_CAPABILITY_SERVO       0x08

#define ANALOG                     0x02 // analog pin in analogInput mode
#define PWM                        0x03 // digital pin in PWM output mode
#define SERVO                      0x04 // digital pin in Servo output mode

/******************************************************
 *              Device Variable Definitions
 ******************************************************/
Servo servos[TOTAL_PINS_NUM];

//for BLEController
byte pins_mode[TOTAL_PINS_NUM];
byte pins_state[TOTAL_PINS_NUM];
byte pins_pwm[TOTAL_PINS_NUM];
byte pins_servo[TOTAL_PINS_NUM];

static byte queryDone = false;
static uint8_t reback_pin = 0;
static uint8_t status_check_flag = 1;

static uint8_t duo_pin[TOTAL_PINS_NUM] = {D0, D1, D2, D3, D4, D5, D6, D7, D8, D9, D10, D11, D12, D13, D14, D15, D16, D17};

/******************************************************
 *               BLE Variable Definitions
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

// BLE peripheral advertising data
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

static uint8_t characteristic1_data[TXRX_BUF_LEN] = { 0x01 };
static uint8_t characteristic2_data[TXRX_BUF_LEN] = { 0x00 };

static btstack_timer_source_t status_check;
static btstack_timer_source_t pin_status_back;

 /******************************************************
 *               Function Definitions
 ******************************************************/
void deviceConnectedCallback(BLEStatus_t status, uint16_t handle) {
  switch (status) {
    case BLE_STATUS_OK:
      Serial.println("Device connected!");
      break;
    default: break;
  }
}

void deviceDisconnectedCallback(uint16_t handle) {
  Serial.println("Disconnected.");
}

void reportPinDigitalData(byte pin) {
  //Serial.println("reportPinDigitalData");
  byte state = digitalRead(duo_pin[pin]);
  byte mode = pins_mode[pin];
  byte buf[] = {'G', pin, mode, state};
  memcpy(characteristic2_data, buf, 4);
  ble.sendNotify(character2_handle, characteristic2_data, 4);
}

void reportPinPWMData(byte pin) {
  //Serial.println("reportPinPWMData");
  byte value = pins_pwm[pin];
  byte mode = pins_mode[pin];
  byte buf[] = {'G', pin, mode, value};
  memcpy(characteristic2_data, buf, min(4,TXRX_BUF_LEN));
  ble.sendNotify(character2_handle, characteristic2_data, min(4,TXRX_BUF_LEN));
}

void reportPinServoData(byte pin) {
  //Serial.println("reportPinServoData");
  byte value = pins_servo[pin];
  byte mode = pins_mode[pin];
  byte buf[] = {'G', pin, mode, value};
  memcpy(characteristic2_data, buf, min(4,TXRX_BUF_LEN));
  ble.sendNotify(character2_handle, characteristic2_data, min(4,TXRX_BUF_LEN));
}

void reportPinCapability(byte pin) {
  //Serial.println("reportPinCapability");
  byte buf[] = {'P', pin, 0x00};
  byte pin_cap = 0;

  if (IS_PIN_DIGITAL(pin)) pin_cap |= PIN_CAPABILITY_DIGITAL;
  if (IS_PIN_ANALOG(pin)) pin_cap |= PIN_CAPABILITY_ANALOG;
  if (IS_PIN_PWM(pin)) pin_cap |= PIN_CAPABILITY_PWM;
  if (IS_PIN_SERVO(pin)) pin_cap |= PIN_CAPABILITY_SERVO;

  buf[2] = pin_cap;
//  Serial.print("report: ");
//  for (uint8_t index = 0; index < 3; index++) 
//      Serial.print(buf[index], HEX);
//      Serial.print(" ");
//  }
//  Serial.println(" ");
  memcpy(characteristic2_data, buf, 3);
  ble.sendNotify(character2_handle, characteristic2_data, 3);
}

void sendCustomData(uint8_t *buf, uint8_t len) {
  //Serial.println("sendCustomData");
  uint8_t data[20] = "Z";

  memcpy(&data[1], buf, len);
  memcpy(characteristic2_data, data, len + 1 );
  ble.sendNotify(character2_handle, characteristic2_data, len + 1);
}

byte reportDigitalInput() {
  static byte pin = 0;
  byte report = 0;

  if (!IS_PIN_DIGITAL(pin)) {
    pin++;
    if (pin >= TOTAL_PINS_NUM) pin = 0;
    return 0;
  }

  if (pins_mode[pin] == INPUT) {
    byte current_state = digitalRead(duo_pin[pin]);
    if (pins_state[pin] != current_state) {
      pins_state[pin] = current_state;
      byte buf[] = {'G', pin, INPUT, current_state};
      memcpy(characteristic2_data, buf, 4);
      ble.sendNotify(character2_handle, characteristic2_data, 4);

      report = 1;
    }
  }
  pin++;
  if (pin >= TOTAL_PINS_NUM) pin = 0;

  return report;
}

byte reportPinAnalogData() {
  static byte pin = 0;
  byte report = 0;

  if (!IS_PIN_DIGITAL(pin)) {
    pin++;
    if (pin >= TOTAL_PINS_NUM) pin = 0;
    return 0;
  }

  if (pins_mode[pin] == ANALOG) {
    uint16_t value = analogRead(duo_pin[pin]);
    byte value_lo = value;
    byte value_hi = value >> 8;

    byte mode = pins_mode[pin];
    mode = (value_hi << 4) | mode;

    byte buf[] = {'G', pin, mode, value_lo};
    memcpy(characteristic2_data, buf, 4);
    ble.sendNotify(character2_handle, characteristic2_data, 4);
  }

  pin++;
  if (pin >= TOTAL_PINS_NUM) pin = 0;

  return report;
}

static void status_check_handle(btstack_timer_source_t *ts) {
  if (status_check_flag) {
    byte input_data_pending = reportDigitalInput();
    if (input_data_pending) {
      // reset
      ble.setTimer(ts, 20);
      ble.addTimer(ts);
      return;
    }
    reportPinAnalogData();
  }
  // reset
  ble.setTimer(ts, 20);
  ble.addTimer(ts);
}

void pin_status_back_handle(btstack_timer_source_t *ts) {
  if (reback_pin < TOTAL_PINS_NUM) {
    reportPinCapability(reback_pin);
    if ( (pins_mode[reback_pin] == INPUT) || (pins_mode[reback_pin] == OUTPUT) )
      reportPinDigitalData(reback_pin);
    else if (pins_mode[reback_pin] == PWM)
      reportPinPWMData(reback_pin);
    else if (pins_mode[reback_pin] == SERVO)
      reportPinServoData(reback_pin);

    reback_pin++;
    // reset
    ble.setTimer(ts, 100);
    ble.addTimer(ts);
  }
  else {
    queryDone = true;
    uint8_t str[] = "ABC";
    sendCustomData(str, 3);
        
    pin_status_back.process = NULL;
    ble.removeTimer(&pin_status_back);
  }  
}

int gattWriteCallback(uint16_t value_handle, uint8_t *buf, uint16_t size) {
  byte len;
  byte pin;
  byte mode;
  byte value;
  byte buf_tx[5];
    
  if (character1_handle == value_handle) {
    memcpy(characteristic1_data, buf, size);
    Serial.print("value: ");
    for (uint8_t index = 0; index < size; index++) {
      Serial.print(characteristic1_data[index], HEX);
      Serial.print(" ");
    }
    Serial.println(" ");
    //Process the data
    switch (buf[0]) {
      case 'V': //query protocol version
        buf_tx[0] = 'V';
        buf_tx[1] = 0x00;
        buf_tx[2] = 0x00;
        buf_tx[3] = 0x01;
        memcpy(characteristic2_data, buf_tx, 4);
        ble.sendNotify(character2_handle, characteristic2_data, 4);   
        break;      
      case 'C': // query board total pin count
        buf_tx[0] = 'C';
        buf_tx[1] = TOTAL_PINS_NUM;
        memcpy(characteristic2_data, buf_tx, 2);
        ble.sendNotify(character2_handle, characteristic2_data, 2);   
        break;                  
      case 'M': // query pin mode
        buf_tx[0] = 'M';
        buf_tx[1] = buf[1];
        buf_tx[2] = pins_mode[ buf[2] ];
        memcpy(characteristic2_data, buf_tx, 3);
        ble.sendNotify(character2_handle, characteristic2_data, 3);  
        break;                  
      case 'S': // query pin mode
        pin = buf[1];
        mode = buf[2];
        if (IS_PIN_SERVO(pin) && mode != SERVO && servos[PIN_TO_SERVO(pin)].attached()) 
          servos[PIN_TO_SERVO(pin)].detach();
        /* ToDo: check the mode is in its capability or not */
        /* assume always ok */
        if (mode != pins_mode[pin]) {   
          pins_mode[pin] = mode;
          if (mode == OUTPUT) {
            pinMode(duo_pin[pin], OUTPUT);
            digitalWrite(duo_pin[pin], LOW);
            pins_state[pin] = LOW;
          }
          else if (mode == INPUT) {
            pinMode(duo_pin[pin], INPUT);
            digitalWrite(duo_pin[pin], HIGH);
            pins_state[pin] = HIGH;
          }
          else if (mode == ANALOG) {
            if (IS_PIN_ANALOG(pin)) {
              //pinMode(duo_pin[pin], AN_INPUT);
            }
          }
          else if (mode == PWM) {
            if (IS_PIN_PWM(pin)) {
              pinMode(duo_pin[PIN_TO_PWM(pin)], OUTPUT);
              analogWrite(duo_pin[PIN_TO_PWM(pin)], 0);
              pins_pwm[pin] = 0;
              pins_mode[pin] = PWM;
            }
          }
          else if (mode == SERVO) {
            if (IS_PIN_SERVO(pin)) {
              pins_servo[pin] = 0;
              pins_mode[pin] = SERVO;
              if (!servos[PIN_TO_SERVO(pin)].attached())
                servos[PIN_TO_SERVO(pin)].attach(duo_pin[PIN_TO_DIGITAL(pin)]);
            }
          }
        }
        //if (mode == ANALOG)
        //  reportPinAnalogData(pin);
        if ( (mode == INPUT) || (mode == OUTPUT) ) {
          reportPinDigitalData(pin);
        }
        else if (mode == PWM) {
          reportPinPWMData(pin);
        }
        else if (mode == SERVO) {
           reportPinServoData(pin);
        }
        break;                  
      case 'G': // query pin data
        pin = buf[1];
        reportPinDigitalData(pin);
        break;              
      case 'T': // set pin digital state
        pin = buf[1];
        value = buf[2];
        digitalWrite(duo_pin[pin], value);
        reportPinDigitalData(pin);
        break;              
      case 'N': // set PWM
        pin = buf[1];
        value = buf[2];
        analogWrite(duo_pin[PIN_TO_PWM(pin)], value);
        pins_pwm[pin] = value;
        reportPinPWMData(pin);
        break;                  
      case 'O': // set Servo
        pin = buf[1];
        value = buf[2];
        if (IS_PIN_SERVO(pin))
          servos[PIN_TO_SERVO(pin)].write(value);
        pins_servo[pin] = value;
        reportPinServoData(pin);
        break;                  
      case 'A': // query all pin status
        reback_pin = 0;
        // set one-shot timer
        pin_status_back.process = &pin_status_back_handle;
        ble.setTimer(&pin_status_back, 100);//2000ms
        ble.addTimer(&pin_status_back);
        break;
      case 'P': // query pin capability
        pin = buf[1];
        reportPinCapability(pin);  
        break;
      case 'Z':
        len = buf[1];
        Serial.println("->");
        Serial.print("Received: ");
        Serial.print(len);
        Serial.println(" byte(s)");
        Serial.print(" Hex: ");
        for (int i = 0;i < len;i++)
          Serial.print(buf[i+2], HEX);
        Serial.println();
        break;
      default: break;
    }
  }
  status_check_flag = 1;
}

void setup() {
  Serial.begin(9600);
  delay(5000);
  Serial.println("BLEController demo.");
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
  character1_handle = ble.addCharacteristicDynamic(service1_tx_uuid, ATT_PROPERTY_NOTIFY|ATT_PROPERTY_WRITE|ATT_PROPERTY_WRITE_WITHOUT_RESPONSE, characteristic1_data, TXRX_BUF_LEN);
  character2_handle = ble.addCharacteristicDynamic(service1_rx_uuid, ATT_PROPERTY_NOTIFY, characteristic2_data, TXRX_BUF_LEN);

  // Set BLE advertising parameters
  ble.setAdvertisementParams(&adv_params);

  // Set BLE advertising data
  ble.setAdvertisementData(sizeof(adv_data), adv_data);

  // BLE peripheral starts advertising now.
  ble.startAdvertising();

  /* Default all to digital input */
  for (int pin = 0; pin < TOTAL_PINS_NUM; pin++) {
    // Set pin to input with internal pull up
    pinMode(duo_pin[pin], INPUT);
    digitalWrite(duo_pin[pin], HIGH);

    // Save pin mode and state
    pins_mode[pin] = INPUT;
    pins_state[pin] = LOW;
  }

  // set one-shot timer
  status_check.process = &status_check_handle;
  ble.setTimer(&status_check, 100);//100ms
  ble.addTimer(&status_check);
    
  Serial.println("BLE start advertising.");
}

void loop() {
    
}
