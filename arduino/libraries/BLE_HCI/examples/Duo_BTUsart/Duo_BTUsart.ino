
/* Includes ------------------------------------------------------------------*/
#include "application.h"
#include "hci.h"
#include "hci_cmd.h"

SYSTEM_MODE(MANUAL);
//SYSTEM_MODE(AUTOMATIC);

int led1 = D7;

const uint8_t adv_data[7]={0x01,0x08,0x20,0x03,0x02,0x01,0x06};
const uint8_t adv_params[19]={0x01,0x06,0x20,0x0F,0xA0,0x00,0xA0,0x00,0x00,0x00,0x00,0x01,0x01,0x01,0x01,0x01,0x01,0x03,0x00 };
const uint8_t adv_enable[5]={0x01,0x0A,0x20,0x01,0x01};
const uint8_t adv_disable[5]={0x01,0x0A,0x20,0x01,0x00};

/* hci interface ------------------------------------------------------*/
void packetHandler(uint8_t type, uint8_t *buf, uint16_t len)
{
    Serial.print("Type : ");  
    Serial.println(type, HEX);

    Serial.print("Packet : ");  
    for(unsigned int index=0; index<len; index++)
    {
        Serial.print(buf[index], HEX); 
        Serial.print(" ");
    }
    Serial.println(""); 
}

void setup()
{
    pinMode(led1, OUTPUT);
    digitalWrite(led1, HIGH);

    Serial.begin(115200);
    Serial.println("start");  
    
    hci_begin();
    hci_registerPacketHandler(packetHandler);
    
    delay(5000);

    hci_sendPacket(adv_params, sizeof(adv_params));
    hci_sendPacket(adv_data, sizeof(adv_data));
    hci_sendPacket(adv_enable, sizeof(adv_enable));   
    
    digitalWrite(led1, LOW);   
}

void loop()
{
    //do nothing.
}

