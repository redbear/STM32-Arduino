
#ifndef HCI_H_
#define HCI_H_

#include "application.h"

// packet header sizes
#define HCI_CMD_HEADER_SIZE          3
#define HCI_ACL_HEADER_SIZE   	     4
#define HCI_SCO_HEADER_SIZE  	     3
#define HCI_EVENT_HEADER_SIZE        2


#define HCI_EVENT_PAYLOAD_SIZE     255
#define HCI_CMD_PAYLOAD_SIZE       255
#define HCI_ACL_PAYLOAD_SIZE       255

// packet buffer sizes
#define HCI_EVENT_BUFFER_SIZE      (HCI_EVENT_HEADER_SIZE + HCI_EVENT_PAYLOAD_SIZE)
#define HCI_CMD_BUFFER_SIZE        (HCI_CMD_HEADER_SIZE   + HCI_CMD_PAYLOAD_SIZE)
#define HCI_ACL_BUFFER_SIZE        (HCI_ACL_HEADER_SIZE   + HCI_ACL_PAYLOAD_SIZE)
    
// size of hci buffers, big enough for command, event, or acl packet without H4 packet type
// @note cmd buffer is bigger than event buffer
#ifdef HCI_PACKET_BUFFER_SIZE
    #if HCI_PACKET_BUFFER_SIZE < HCI_ACL_BUFFER_SIZE
        #error HCI_PACKET_BUFFER_SIZE must be equal or larger than HCI_ACL_BUFFER_SIZE
    #endif
    #if HCI_PACKET_BUFFER_SIZE < HCI_CMD_BUFFER_SIZE
        #error HCI_PACKET_BUFFER_SIZE must be equal or larger than HCI_CMD_BUFFER_SIZE
    #endif
#else
    #if HCI_ACL_BUFFER_SIZE > HCI_CMD_BUFFER_SIZE
        #define HCI_PACKET_BUFFER_SIZE HCI_ACL_BUFFER_SIZE
    #else
        #define HCI_PACKET_BUFFER_SIZE HCI_CMD_BUFFER_SIZE
    #endif
#endif

// BNEP may uncompress the IP Header by 16 bytes
#ifdef HAVE_BNEP
#define HCI_INCOMING_PRE_BUFFER_SIZE (16 - HCI_ACL_HEADER_SIZE - 4)
#endif 
#ifndef HCI_INCOMING_PRE_BUFFER_SIZE
    #define HCI_INCOMING_PRE_BUFFER_SIZE 0
#endif

typedef void (*packetHandler_t)(uint8_t type, uint8_t *buf, uint16_t len);

int  hci_begin(void);
int  hci_sendPacket(const uint8_t *packet, uint8_t len);
void hci_registerPacketHandler(packetHandler_t handler);


#endif
