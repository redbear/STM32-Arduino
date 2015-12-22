

#include "hci.h"
#include "hci_cmd.h"

#define  PACKET_RECEIVE_TIMEOUT  0x0000FFFF  

typedef enum {
  PKT_TYPE = 1,
  PKT_HEAD_ACL,
  PKT_HEAD_EVENT,
  PKT_PAYLOAD,
  PKT_FINISHED
}pkt_status_t;

static packetHandler_t packetHandler;

static pkt_status_t pkt_status = PKT_TYPE;
static uint8_t      hci_packet_prefixed[HCI_INCOMING_PRE_BUFFER_SIZE + HCI_PACKET_BUFFER_SIZE];
static uint8_t*     hci_packet = &hci_packet_prefixed[HCI_INCOMING_PRE_BUFFER_SIZE];
static uint8_t      packet_offset;
static uint16_t     packet_payload_len;
static uint32_t     timeout;

void receiveHandler(void)
{
    while(BTUsart.available() > 0) {
        if(pkt_status == PKT_TYPE) {
            packet_offset = 0;
            packet_payload_len = 0;
            timeout = 0;
            if(BTUsart.available()) {
                hci_packet[packet_offset] = BTUsart.read();
                packet_offset++;
                // Check packet type.
                if(hci_packet[0] == HCI_ACL_DATA_PACKET)
                    pkt_status = PKT_HEAD_ACL;
                else if(hci_packet[0] == HCI_EVENT_PACKET)
                    pkt_status = PKT_HEAD_EVENT;
                else {
                    // Invalid packet type.
                    pkt_status = PKT_TYPE;
                    packet_offset = 0;
                }
            }
        }

        if(pkt_status == PKT_HEAD_ACL) {
            // Wait for received ACL HEADER.
            if(BTUsart.available() >= HCI_ACL_HEADER_SIZE) {
                uint8_t index;
                for(index=0; index<HCI_ACL_HEADER_SIZE; index++) {
                    hci_packet[packet_offset] = BTUsart.read();
                    packet_offset++;
                }

                packet_payload_len = (hci_packet[packet_offset-1]<<8) + hci_packet[packet_offset-2];
                pkt_status = PKT_PAYLOAD;
                timeout = 0;
            }
            else {
                timeout++;
                if(timeout >= PACKET_RECEIVE_TIMEOUT){
                    if(BTUsart.available())
                        BTUsart.read();
                    pkt_status=PKT_TYPE; 
                }  
            }
        }

        if(pkt_status == PKT_HEAD_EVENT) {
            if(BTUsart.available() >= HCI_EVENT_HEADER_SIZE) {
                uint8_t index;
                for(index=0; index<HCI_EVENT_HEADER_SIZE; index++) {
                    hci_packet[packet_offset] = BTUsart.read();
                    packet_offset++;
                }

                packet_payload_len = hci_packet[packet_offset-1];
                pkt_status = PKT_PAYLOAD;
                timeout = 0;
            }
            else {
                timeout++;
                if(timeout >= PACKET_RECEIVE_TIMEOUT){
                    if(BTUsart.available())
                        BTUsart.read();
                    pkt_status=PKT_TYPE; 
                }  
            }
        }

        if(pkt_status == PKT_PAYLOAD) {
            if(BTUsart.available() >= packet_payload_len){
                uint8_t index;
                for(index=0; index<packet_payload_len; index++) {
                    hci_packet[packet_offset] = BTUsart.read();
                    packet_offset++;
                }
                pkt_status = PKT_FINISHED;
                timeout = 0;
            }
            else {
                timeout++;
                if(timeout >= PACKET_RECEIVE_TIMEOUT){
                    if(BTUsart.available())
                        BTUsart.read();
                    pkt_status=PKT_TYPE; 
                }  
            }
        }

        if(pkt_status == PKT_FINISHED)
        {
            BTUsart.setRTS(HIGH);
            
            if(packetHandler != NULL)
                packetHandler(hci_packet[0], &hci_packet[1], (packet_offset-1));
            
            pkt_status = PKT_TYPE;
            packet_offset = 0;

            BTUsart.setRTS(LOW);
        }
    } 
    BTUsart.restartTransmit();
}

int hci_begin(void)
{
    pkt_status         = PKT_TYPE;
    packet_offset      = 0;
    packet_payload_len = 0;
    
    WiFi.on();
    
    BTUsart.begin(115200);
    BTUsart.registerReceiveHandler(receiveHandler);
    
    return BTUsart.downloadFirmware();     
}

int hci_sendPacket(const uint8_t *packet, uint8_t len)
{
    return BTUsart.write(packet, len);
}

void hci_registerPacketHandler(packetHandler_t handler)
{
    packetHandler = handler;
}
