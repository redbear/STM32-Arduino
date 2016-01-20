#include <BTstack.h>
#include <btstack_api.h>
#include "Arduino.h"

//SYSTEM_MODE(AUTOMATIC);//connect to cloud
SYSTEM_MODE(MANUAL);//do not connect to cloud

#define DIGITAL_OUT_PIN            D2
#define LED_PIN                    D7
#define DIGITAL_IN_PIN             A4
#define PWM_PIN                    D3
#define ANALOG_IN_PIN              A5
#define SERVO_PIN                  D4
#define CHECK_STATUS_TIME          200

#define DEVICE_NAME                "BLE_SimpleControls"

#define CHARACTERISTIC1_MAX_LEN    4
#define CHARACTERISTIC2_MAX_LEN    4

static uint8_t  appearance[2] = {0x00, 0x02};
static uint8_t  change[2]     = {0x00, 0x00};
static uint8_t  conn_param[8] = {0x28, 0x00, 0x90, 0x01, 0x00, 0x00, 0x90, 0x01};

static uint16_t character1_handle = 0x0000;
static uint16_t character2_handle = 0x0000;

static uint8_t characteristic1_data[CHARACTERISTIC1_MAX_LEN]={0x00};
static uint8_t characteristic2_data[CHARACTERISTIC2_MAX_LEN]={0x00};

static boolean analog_enabled = false;
static byte old_state         = LOW;
static timer_source_t           characteristic2;
Servo                          myservo;

//Advertising Data.
const uint8_t adv_data[]={0x02,0x01,0x06, 0x11,0x07,0x1e,0x94,0x8d,0xf1,0x48,0x31,0x94,0xba,0x75,0x4c,0x3e,0x50,0x00,0x00,0x3d,0x71};

void deviceConnectedCallback(BLEStatus status, BLEDevice *device) {
    switch (status){
        case BLE_STATUS_OK:
            Serial.println("Device connected!");
            break;
        default:
            break;
    }
}

void deviceDisconnectedCallback(BLEDevice * device){
    Serial.println("Disconnected.");
}

uint16_t gattReadCallback(uint16_t value_handle, uint8_t * buffer, uint16_t buffer_size){
    // Note:At here,ignore paramt "buffer_size",just copy local data to buffer, and return the length of local data.
    // Note:The CCCD handle is characterx_handle+1.
    uint8_t characteristic_len=0;
    
    Serial.print("Read value handler: ");
    Serial.println(value_handle, HEX);  
    if(character1_handle == value_handle)
    {   
        Serial.println("Character1 read:");
        memcpy(buffer, characteristic1_data, CHARACTERISTIC1_MAX_LEN);
        characteristic_len = CHARACTERISTIC1_MAX_LEN;
    }
    else if(character2_handle == value_handle)
    {
        Serial.println("Character2 read:");
        memcpy(buffer, characteristic2_data, CHARACTERISTIC2_MAX_LEN);
        characteristic_len = CHARACTERISTIC2_MAX_LEN;    
    }
    return characteristic_len;
}

int gattWriteCallback(uint16_t value_handle, uint8_t *buffer, uint16_t size){
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
        //Process the data
        if (characteristic1_data[0] == 0x01)  // Command is to control digital out pin
        {
            if (characteristic1_data[1] == 0x01)
            {
                digitalWrite(DIGITAL_OUT_PIN, HIGH);
                digitalWrite(LED_PIN, HIGH);
            }
            else
            {
                digitalWrite(DIGITAL_OUT_PIN, LOW);
                digitalWrite(LED_PIN, LOW);
            }
        }
        else if (characteristic1_data[0] == 0xA0) // Command is to enable analog in reading
        {
            if (characteristic1_data[1] == 0x01)
                analog_enabled = true;
            else
                analog_enabled = false;
        }
        else if (characteristic1_data[0] == 0x02) // Command is to control PWM pin
        {
            analogWrite(PWM_PIN, characteristic1_data[1]);
        }
        else if (characteristic1_data[0] == 0x03)  // Command is to control Servo pin
        {
            myservo.write(characteristic1_data[1]);
        }
        else if (characteristic1_data[0] == 0x04)
        {
            analog_enabled = false;
            myservo.write(0);
            analogWrite(PWM_PIN, 0);
            digitalWrite(DIGITAL_OUT_PIN, LOW);
            digitalWrite(LED_PIN, LOW);
            old_state = LOW;
        }        
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

static void  characteristic2_notify(struct timer *ts)
{
    uint8_t buf[3];
    if (digitalRead(DIGITAL_IN_PIN) != old_state)
    {   // If digital in changes, report the state
        old_state = digitalRead(DIGITAL_IN_PIN);
        if (digitalRead(DIGITAL_IN_PIN) == HIGH)
        {
            Serial.print("DIGITAL_IN_PIN: HIGH");
            buf[0] = (0x0A);
            buf[1] = (0x01);
            buf[2] = (0x00);
            att_server_notify(character2_handle, buf, 3);
        }
        else
        {
            Serial.print("DIGITAL_IN_PIN:LOW");
            buf[0] = (0x0A);
            buf[1] = (0x00);
            buf[2] = (0x00);
            att_server_notify(character2_handle, buf, 3);
        }
    }
    else if(analog_enabled)
    {   // Read and send out
        uint16_t value = analogRead(ANALOG_IN_PIN);
        buf[0] = (0x0B);
        buf[1] = (value >> 8);
        buf[2] = (value);
        att_server_notify(character2_handle, buf, 3);
    }
    // reset time.
    run_loop_set_timer(ts, CHECK_STATUS_TIME);
    run_loop_add_timer(ts);
}

void setup() {

    pinMode(D7, OUTPUT);
    Serial.begin(115200);
    delay(5000);
    
    // startup Bluetooth
    bd_addr_t dummy = { 1,2,3,3,2,1};
    BTstack.setPublicBdAddr(dummy);
    BTstack.setup();
    
    // set callbacks
    BTstack.setBLEDeviceConnectedCallback(deviceConnectedCallback);
    BTstack.setBLEDeviceDisconnectedCallback(deviceDisconnectedCallback);
    BTstack.setGATTCharacteristicRead(gattReadCallback);
    BTstack.setGATTCharacteristicWrite(gattWriteCallback);

    // setup GATT Database
    BTstack.addGATTService(0x1800);
    BTstack.addGATTCharacteristic(0x2A00, ATT_PROPERTY_READ | ATT_PROPERTY_WRITE, (uint8_t*)DEVICE_NAME, sizeof(DEVICE_NAME));
    BTstack.addGATTCharacteristic(0x2A01, ATT_PROPERTY_READ, appearance, sizeof(appearance));
    BTstack.addGATTCharacteristic(0x2A04, ATT_PROPERTY_READ, conn_param, sizeof(conn_param));
    BTstack.addGATTService(0x1801);
    BTstack.addGATTCharacteristic(0x2A05, ATT_PROPERTY_INDICATE, change, sizeof(change));
    
    BTstack.addGATTService(new UUID("713d0000-503e-4c75-ba94-3148f18d941e"));
    character1_handle = BTstack.addGATTCharacteristicDynamic(new UUID("713d0003-503e-4c75-ba94-3148f18d941e"), ATT_PROPERTY_WRITE | ATT_PROPERTY_WRITE_WITHOUT_RESPONSE, characteristic1_data, CHARACTERISTIC1_MAX_LEN, 0);
    character2_handle = BTstack.addGATTCharacteristicDynamic(new UUID("713d0002-503e-4c75-ba94-3148f18d941e"), ATT_PROPERTY_NOTIFY, characteristic2_data, CHARACTERISTIC2_MAX_LEN, 1);
 
    // set ble advertising.
    BTstack.setAdvData(sizeof(adv_data), adv_data);
    BTstack.startAdvertising();
    Serial.println("Start advertising");

    pinMode(DIGITAL_OUT_PIN, OUTPUT);
    pinMode(LED_PIN, OUTPUT);
    pinMode(DIGITAL_IN_PIN, INPUT_PULLUP);
    pinMode(PWM_PIN, OUTPUT);

    digitalWrite(DIGITAL_IN_PIN, HIGH);
    myservo.attach(SERVO_PIN);
    myservo.write(0);
    
    // set one-shot timer
    characteristic2.process = &characteristic2_notify;
    run_loop_set_timer(&characteristic2, CHECK_STATUS_TIME);
    run_loop_add_timer(&characteristic2);
}

void loop() {
    // put your main code here, to run repeatedly:
    // Note:Don't do any delay at loop.
    BTstack.loop();
}


