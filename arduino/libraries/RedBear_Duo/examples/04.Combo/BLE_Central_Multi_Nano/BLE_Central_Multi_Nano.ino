
#include "application.h"
#include "ble_nano.h"
#include "aJSON.h"

// your network name also called SSID
char ssid[] = "Duo";
// your network password
char password[] = "password";

// Initialize the Ethernet client library
// with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):

TCPServer server = TCPServer(80);
TCPClient client;

#if defined(ARDUINO) 
SYSTEM_MODE(MANUAL);//do not connect to cloud
#else
SYSTEM_MODE(AUTOMATIC);//connect to cloud
#endif

#define NANO_0_MASK                       0x0001
#define NANO_1_MASK                       0x0002
#define NANO_2_MASK                       0x0004
#define NANO_3_MASK                       0x0008
#define NANO_4_MASK                       0x0010
#define NANO_5_MASK                       0x0020
#define NANO_6_MASK                       0x0040
#define NANO_7_MASK                       0x0080

#define ALL_CONNECTED_MASK   (NANO_0_MASK|NANO_1_MASK|NANO_2_MASK|NANO_3_MASK|NANO_4_MASK|NANO_5_MASK|NANO_6_MASK|NANO_7_MASK)
#define ALL_DISCOVERYED_MASK (NANO_0_MASK|NANO_1_MASK|NANO_2_MASK|NANO_3_MASK|NANO_4_MASK|NANO_5_MASK|NANO_6_MASK|NANO_7_MASK)

/******************************************************
 *                   Enumerations
 ******************************************************/
typedef enum{
  PERIPHERAL_IDLE,
  PERIPHERAL_NANO_0,
  PERIPHERAL_NANO_1,
  PERIPHERAL_NANO_2,
  PERIPHERAL_NANO_3,
  PERIPHERAL_NANO_4,
  PERIPHERAL_NANO_5,
  PERIPHERAL_NANO_6,
  PERIPHERAL_NANO_7,
}PeripheralState_t;

/******************************************************
 *               Variable Definitions
 ******************************************************/

static PeripheralState_t  peripheralState = PERIPHERAL_IDLE;
static uint16_t       peripheralConnectedState = 0x0000;
static uint16_t       peripheralDiscoveyState  = 0X0000;
static uint8_t is_connecting_flag = 0;

static char rx_buf[255];
static uint8_t rx_len;

static uint8_t is_nano_ok = 0;
static uint8_t is_wifi_connected = 0;
/******************************************************
 *               Function Definitions
 ******************************************************/
 void printWifiStatus() {
    // print the SSID of the network you're attached to:
    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());

    // print your WiFi shield's IP address:
    IPAddress ip = WiFi.localIP();
    Serial.print("IP Address: ");
    Serial.println(ip);

    // print the received signal strength:
    long rssi = WiFi.RSSI();
    Serial.print("signal strength (RSSI):");
    Serial.print(rssi);
    Serial.println(" dBm");
}

void send_status(uint8_t *buf)
{
    aJsonObject *root = aJson.createObject();
    aJsonObject *temp = aJson.createObject();

    aJson.addItemToObject(temp, "ID", aJson.createItem(buf[0]));
    aJson.addItemToObject(temp, "OpCode", aJson.createItem(buf[1]));
    aJson.addItemToObject(temp, "R", aJson.createItem(buf[2]));
    aJson.addItemToObject(temp, "G", aJson.createItem(buf[3]));
    aJson.addItemToObject(temp, "B", aJson.createItem(buf[4]));
    aJson.addItemToObject(root, "RGB", temp);
    
    const char *p = aJson.print(root);

    aJson.deleteItem(root);
    aJson.deleteItem(temp);
    
    client.println(p);
    RGB.color(255, 255, 255);   
}

void parseJson(char *jsonString)
{
    aJsonObject* root;
    root = aJson.parse(jsonString);
    if(NULL != root)
    {
        aJsonObject* rgb_object = aJson.getObjectItem(root, "RGB");
        if(rgb_object != NULL)
        {
            aJsonObject* id = aJson.getObjectItem(rgb_object, "ID");
            aJsonObject* OpCode = aJson.getObjectItem(rgb_object, "OpCode");
            aJsonObject* r = aJson.getObjectItem(rgb_object, "R");
            aJsonObject* g = aJson.getObjectItem(rgb_object, "G");
            aJsonObject* b = aJson.getObjectItem(rgb_object, "B");

            if( (id!=NULL) && (OpCode!=NULL) && (r!=NULL) && (g!=NULL) && (b!=NULL) )
            {
                if(OpCode->valueint == 1)
                {   // Write command.
                    uint8_t buf[5] = {0xA5, id->valueint, r->valueint, g->valueint, b->valueint};
                    if(is_nano_ok)
                        nano_write(id->valueint, buf, 5);
                }
                else if(OpCode->valueint == 2)
                {   // Read command.
                    nano_read(id->valueint);
                }
            }
            aJson.deleteItem(id);
            aJson.deleteItem(OpCode);
            aJson.deleteItem(r);
            aJson.deleteItem(g);
            aJson.deleteItem(b);
        }       
        aJson.deleteItem(root);
        aJson.deleteItem(rgb_object);
    }
    else
    {
        Serial.println("NULL Json"); 
    }
    RGB.color(255, 255, 255);   
}

uint32_t ble_advdata_decode(uint8_t type, uint8_t advdata_len, uint8_t *p_advdata, uint8_t *len, uint8_t *p_field_data)
{
    uint8_t index=0;
    uint8_t field_length, field_type;

    while(index<advdata_len)
    {
        field_length = p_advdata[index];
        field_type   = p_advdata[index+1];
        if(field_type == type)
        {
            memcpy(p_field_data, &p_advdata[index+2], (field_length-1));
            *len = field_length - 1;
            return 0;
        }
        index += field_length + 1;
    }
    return 1;
}

void reportCallback(advertisementReport_t *report)
{
    uint8_t index;

    Serial.println("reportCallback: ");
    Serial.print("The advEventType: ");
    Serial.println(report->advEventType, HEX);
    Serial.print("The peerAddrType: ");
    Serial.println(report->peerAddrType, HEX);
    Serial.print("The peerAddr: ");
    for(index=0; index<6; index++)
    {
        Serial.print(report->peerAddr[index], HEX);
        Serial.print(" ");
    }
    Serial.println(" ");

    Serial.print("The rssi: ");
    Serial.println(report->rssi, DEC);

    Serial.print("The ADV data: ");
    for(index=0; index<report->advDataLen; index++)
    {
        Serial.print(report->advData[index], HEX);
        Serial.print(" ");
    }
    Serial.println(" ");

    uint8_t len;
    uint8_t adv_name[31];
    if(0x00 == ble_advdata_decode(0x08, report->advDataLen, report->advData, &len, adv_name))
    {
        Serial.print("  The length of Short Local Name : ");
        Serial.println(len, HEX);
        Serial.print("  The Short Local Name is        : ");
        Serial.println((const char *)adv_name);
        if(0x00 == memcmp(adv_name, "Demo_Nano_0", min(11, len)))
        {
            Serial.println("Find nano_0...");
            //Make sure we need to connect to nano_0 and nano_0 has not been connected.
            if( (is_connecting_flag == 0) && (ALL_CONNECTED_MASK & NANO_0_MASK)==NANO_0_MASK && ((peripheralConnectedState&NANO_0_MASK) == 0x00) )
            {
                Serial.println("Connecting to nano_0...");
                ble.stopScanning();

                peripheralState = PERIPHERAL_NANO_0;
                nano_setPeerAddr(0, report->peerAddr, BD_ADDR_TYPE_LE_RANDOM);
                nano_connect(0);
                is_connecting_flag = 1;
            }
        }
        else if( 0x00 == memcmp(adv_name, "Demo_Nano_1", min(len,11)) )
        {
            Serial.println("Find nano_1...");
            if( (is_connecting_flag == 0) && ((ALL_CONNECTED_MASK & NANO_1_MASK)==NANO_1_MASK) && ((peripheralConnectedState&NANO_1_MASK) == 0x00) )
            {
                Serial.println("Connecting to nano_1...");
                ble.stopScanning();

                peripheralState = PERIPHERAL_NANO_1;
                nano_setPeerAddr(1, report->peerAddr, BD_ADDR_TYPE_LE_RANDOM);
                nano_connect(1);
                is_connecting_flag = 1;
            }
        }
        else if( 0x00 == memcmp(adv_name, "Demo_Nano_2", min(len,11)) )
        {
            Serial.println("Find nano_2...");
            if( (is_connecting_flag == 0) && ((ALL_CONNECTED_MASK & NANO_2_MASK)==NANO_2_MASK) && ((peripheralConnectedState&NANO_2_MASK) == 0x00) )
            {
                Serial.println("Connecting to nano_2...");
                ble.stopScanning();

                peripheralState = PERIPHERAL_NANO_2;
                nano_setPeerAddr(2, report->peerAddr, BD_ADDR_TYPE_LE_RANDOM);
                nano_connect(2);
                is_connecting_flag = 1;
            }
        }
        else if( 0x00 == memcmp(adv_name, "Demo_Nano_3", min(len,11)) )
        {   
            Serial.println("Find nano_3...");
            if( (is_connecting_flag == 0) && ((ALL_CONNECTED_MASK & NANO_3_MASK)==NANO_3_MASK) && ((peripheralConnectedState&NANO_3_MASK) == 0x00) )
            {
                Serial.println("Connecting to nano_3...");
                ble.stopScanning();

                peripheralState = PERIPHERAL_NANO_3;
                nano_setPeerAddr(3, report->peerAddr, BD_ADDR_TYPE_LE_RANDOM);
                nano_connect(3);
                is_connecting_flag = 1;
            }
        }
        else if( 0x00 == memcmp(adv_name, "Demo_Nano_4", min(len,11)) )
        {
            Serial.println("Find nano_4...");
            if( (is_connecting_flag == 0) && ((ALL_CONNECTED_MASK & NANO_4_MASK)==NANO_4_MASK) && ((peripheralConnectedState&NANO_4_MASK) == 0x00) )
            {
                Serial.println("Connecting to nano_4...");
                ble.stopScanning();

                peripheralState = PERIPHERAL_NANO_4;
                nano_setPeerAddr(4, report->peerAddr, BD_ADDR_TYPE_LE_RANDOM);
                nano_connect(4);
                is_connecting_flag = 1;
            }
        }
        else if( 0x00 == memcmp(adv_name, "Demo_Nano_5", min(len,11)) )
        {
            Serial.println("Find nano_5...");
            if( (is_connecting_flag == 0) && ((ALL_CONNECTED_MASK & NANO_5_MASK)==NANO_5_MASK) && ((peripheralConnectedState&NANO_5_MASK) == 0x00) )
            {
                Serial.println("Connecting to nano_5...");
                ble.stopScanning();

                peripheralState = PERIPHERAL_NANO_5;
                nano_setPeerAddr(5, report->peerAddr, BD_ADDR_TYPE_LE_RANDOM);
                nano_connect(5);
                is_connecting_flag = 1;
            }
        }
        else if( 0x00 == memcmp(adv_name, "Demo_Nano_6", min(len,11)) )
        {
            Serial.println("Find nano_6...");
            if( (is_connecting_flag == 0) && ((ALL_CONNECTED_MASK & NANO_6_MASK)==NANO_6_MASK) && ((peripheralConnectedState&NANO_6_MASK) == 0x00) )
            {
                Serial.println("Connecting to nano_6...");
                ble.stopScanning();

                peripheralState = PERIPHERAL_NANO_6;
                nano_setPeerAddr(6, report->peerAddr, BD_ADDR_TYPE_LE_RANDOM);
                nano_connect(6);
                is_connecting_flag = 1;
            }
        }
        else if( 0x00 == memcmp(adv_name, "Demo_Nano_7", min(len,11)) )
        {
              Serial.println("Find nano_7...");
              if( (is_connecting_flag == 0) && ((ALL_CONNECTED_MASK & NANO_7_MASK)==NANO_7_MASK) && ((peripheralConnectedState&NANO_7_MASK) == 0x00) )
              {
                  Serial.println("Connecting to nano_7...");
                  ble.stopScanning();

                  peripheralState = PERIPHERAL_NANO_7;
                  nano_setPeerAddr(7, report->peerAddr, BD_ADDR_TYPE_LE_RANDOM);
                  nano_connect(7);
                  is_connecting_flag = 1;
              }
          }
    }
    else if(0x00 == ble_advdata_decode(0x09, report->advDataLen, report->advData, &len, adv_name))
    {
      
    }
}

void deviceDisconnectedCallback(uint16_t handle){
    Serial.print("Disconnected handle : ");
    Serial.println(handle,HEX);
    RGB.color(255, 0, 0);
    is_nano_ok = 0;
    if(nano_getConnectID(0) == handle)
    {
        Serial.println("PERIPHERAL_NANO_0 disconnected.");
        peripheralConnectedState &= ~NANO_0_MASK;
        peripheralDiscoveyState &= ~NANO_0_MASK;
        nano_stopNotify(0);
        nano_setConnectID(0, INVALID_CONN_HANDLE);
    }
    else if(nano_getConnectID(1) == handle)
    {
        Serial.println("PERIPHERAL_NANO_1 disconnected.");
        peripheralConnectedState &= ~NANO_1_MASK;
        peripheralDiscoveyState &= ~NANO_1_MASK;
        nano_stopNotify(1);
        nano_setConnectID(1, INVALID_CONN_HANDLE);
    }
    else if(nano_getConnectID(2) == handle)
    {
        Serial.println("PERIPHERAL_NANO_2 disconnected.");
        peripheralConnectedState &= ~NANO_2_MASK;
        peripheralDiscoveyState &= ~NANO_2_MASK;
        nano_stopNotify(2);
        nano_setConnectID(2, INVALID_CONN_HANDLE);
    }
    else if(nano_getConnectID(3) == handle)
    {
        Serial.println("PERIPHERAL_NANO_3 disconnected.");
        peripheralConnectedState &= ~NANO_3_MASK;
        peripheralDiscoveyState &= ~NANO_3_MASK;
        nano_stopNotify(3);
        nano_setConnectID(3, INVALID_CONN_HANDLE);
    }
    else if(nano_getConnectID(4) == handle)
    {
        Serial.println("PERIPHERAL_NANO_4 disconnected.");
        peripheralConnectedState &= ~NANO_4_MASK;
        peripheralDiscoveyState &= ~NANO_4_MASK;
        nano_stopNotify(4);
        nano_setConnectID(4, INVALID_CONN_HANDLE);
    }
    else if(nano_getConnectID(5) == handle)
    {
        Serial.println("PERIPHERAL_NANO_5 disconnected.");
        peripheralConnectedState &= ~NANO_5_MASK;
        peripheralDiscoveyState &= ~NANO_5_MASK;
        nano_stopNotify(5);
        nano_setConnectID(5, INVALID_CONN_HANDLE);
    }
    else if(nano_getConnectID(6) == handle)
    {
        Serial.println("PERIPHERAL_NANO_6 disconnected.");
        peripheralConnectedState &= ~NANO_6_MASK;
        peripheralDiscoveyState &= ~NANO_6_MASK;
        nano_stopNotify(6);
        nano_setConnectID(6, INVALID_CONN_HANDLE);
    }
    else if(nano_getConnectID(7) == handle)
    {
        Serial.println("PERIPHERAL_NANO_7 disconnected.");
        peripheralConnectedState &= ~NANO_7_MASK;
        peripheralDiscoveyState &= ~NANO_7_MASK;
        nano_stopNotify(7);
        nano_setConnectID(7, INVALID_CONN_HANDLE);
    }

    if( (is_connecting_flag == 0) && (peripheralConnectedState != ALL_CONNECTED_MASK) )
    {
        Serial.println("Restart scanning.");
        ble.startScanning();
    }
}

void deviceConnectedCallback(BLEStatus_t status, uint16_t handle) {
    switch (status){
        case BLE_STATUS_OK:
            Serial.print("Device connected : ");
            Serial.println(handle, HEX);
            if(peripheralState == PERIPHERAL_NANO_0)
            {
                Serial.println("Connect to PERIPHERAL_NANO_0.");
                peripheralConnectedState |= NANO_0_MASK;
                nano_setConnectID(0, handle);
            }
            else if(peripheralState == PERIPHERAL_NANO_1)
            {
                Serial.println("Connect to PERIPHERAL_NANO_1.");
                peripheralConnectedState |= NANO_1_MASK;
                nano_setConnectID(1, handle);
            }
            else if(peripheralState == PERIPHERAL_NANO_2)
            {
                Serial.println("Connect to PERIPHERAL_NANO_2.");
                peripheralConnectedState |= NANO_2_MASK;
                nano_setConnectID(2, handle);
            }
            else if(peripheralState == PERIPHERAL_NANO_3)
            {
                Serial.println("Connect to PERIPHERAL_NANO_3.");
                  peripheralConnectedState |= NANO_3_MASK;
                nano_setConnectID(3, handle);
            }
            else if(peripheralState == PERIPHERAL_NANO_4)
            {
                Serial.println("Connect to PERIPHERAL_NANO_4.");
                peripheralConnectedState |= NANO_4_MASK;
                nano_setConnectID(4, handle);
            }
            else if(peripheralState == PERIPHERAL_NANO_5)
            {
                Serial.println("Connect to PERIPHERAL_NANO_5.");
                peripheralConnectedState |= NANO_5_MASK;
                nano_setConnectID(5, handle);
            }
            else if(peripheralState == PERIPHERAL_NANO_6)
            {
                Serial.println("Connect to PERIPHERAL_NANO_6.");
                peripheralConnectedState |= NANO_6_MASK;
                nano_setConnectID(6, handle);
            }
            else if(peripheralState == PERIPHERAL_NANO_7)
            {
                Serial.println("Connect to PERIPHERAL_NANO_7.");
                peripheralConnectedState |= NANO_7_MASK;
                nano_setConnectID(7, handle);
            }

            //Check whether all nano have been connected.
            if(peripheralConnectedState == ALL_CONNECTED_MASK)
            {
                Serial.println("ALL NANO Connected.");
                ble.stopScanning();

                if( ((ALL_DISCOVERYED_MASK & NANO_0_MASK)==NANO_0_MASK) && ((peripheralDiscoveyState & NANO_0_MASK)!=NANO_0_MASK) )
                {
                    peripheralState = PERIPHERAL_NANO_0;
                    nano_discoverService(0);
                }
                else if( ((ALL_DISCOVERYED_MASK & NANO_1_MASK)==NANO_1_MASK) && ((peripheralDiscoveyState & NANO_1_MASK)!=NANO_1_MASK) )
                { 
                    peripheralState = PERIPHERAL_NANO_1;
                    nano_discoverService(1);
                }
                else if( ((ALL_DISCOVERYED_MASK & NANO_2_MASK)==NANO_2_MASK) && ((peripheralDiscoveyState & NANO_2_MASK)!=NANO_2_MASK) )
                {
                    peripheralState = PERIPHERAL_NANO_2;
                    nano_discoverService(2);
                }
                else if( ((ALL_DISCOVERYED_MASK & NANO_3_MASK)==NANO_3_MASK) && ((peripheralDiscoveyState & NANO_3_MASK)!=NANO_3_MASK) )
                {
                    peripheralState = PERIPHERAL_NANO_3;
                    nano_discoverService(3);
                }
                else if( ((ALL_DISCOVERYED_MASK & NANO_4_MASK)==NANO_4_MASK) && ((peripheralDiscoveyState & NANO_4_MASK)!=NANO_4_MASK) )
                {
                    peripheralState = PERIPHERAL_NANO_4;
                    nano_discoverService(4);
                }
                else if( ((ALL_DISCOVERYED_MASK & NANO_5_MASK)==NANO_5_MASK) && ((peripheralDiscoveyState & NANO_5_MASK)!=NANO_5_MASK) )
                {
                    peripheralState = PERIPHERAL_NANO_5;
                    nano_discoverService(5);
                }
                else if( ((ALL_DISCOVERYED_MASK & NANO_6_MASK)==NANO_6_MASK) && ((peripheralDiscoveyState & NANO_6_MASK)!=NANO_6_MASK) )
                {
                    peripheralState = PERIPHERAL_NANO_6;
                    nano_discoverService(6);
                }
                else if( ((ALL_DISCOVERYED_MASK & NANO_7_MASK)==NANO_7_MASK) && ((peripheralDiscoveyState & NANO_7_MASK)!=NANO_7_MASK) )
                {
                    peripheralState = PERIPHERAL_NANO_7;
                    nano_discoverService(7);
                }
            }
            else
            {
                Serial.println("Restart scanning.");
                ble.startScanning();
            }
            break;
        default:
            peripheralState = PERIPHERAL_IDLE;
            ble.startScanning();
            break;
    }
    is_connecting_flag = 0;
}

static void discoveredServiceCallback(BLEStatus_t status, uint16_t con_handle, gatt_client_service_t *service)
{
    if(status == BLE_STATUS_OK)
    {
        Serial.println(" ");
        Serial.print("Service start handle: ");
        Serial.println(service->start_group_handle, HEX);
        Serial.print("Service end handle: ");
        Serial.println(service->end_group_handle, HEX);
        Serial.print("Service uuid16: ");
        Serial.println(service->uuid16, HEX);
        uint8_t index;
        Serial.print("The uuid128 : ");
        for(index=0; index<16; index++)
        {
            Serial.print(service->uuid128[index], HEX);
            Serial.print(" ");
        }
        Serial.println(" ");

        if(nano_getConnectID(0) == con_handle)
        {
            nano_discoveredServiceResult(0,service);
        }
        else if(nano_getConnectID(1) == con_handle)
        {
            nano_discoveredServiceResult(1,service);
        }
        else if(nano_getConnectID(2) == con_handle)
        {
            nano_discoveredServiceResult(2,service);
        }
        else if(nano_getConnectID(3) == con_handle)
        {
            nano_discoveredServiceResult(3,service);
        }
        else if(nano_getConnectID(4) == con_handle)
        {
            nano_discoveredServiceResult(4,service);
        }
        else if(nano_getConnectID(5) == con_handle)
        {
            nano_discoveredServiceResult(5,service);
        }
        else if(nano_getConnectID(6) == con_handle)
        {
            nano_discoveredServiceResult(6,service);
        }
        else if(nano_getConnectID(7) == con_handle)
        {
            nano_discoveredServiceResult(7,service);
        }
    }
    else if(status == BLE_STATUS_DONE)
    {
        Serial.println("Discovered service done, start to discover chars of service.");
        if(nano_getConnectID(0) == con_handle)
        {
            nano_discoverCharsOfService(0);
        }
        else if(nano_getConnectID(1) == con_handle)
        {
            nano_discoverCharsOfService(1);
        }
        else if(nano_getConnectID(2) == con_handle)
        {
            nano_discoverCharsOfService(2);
        }
        else if(nano_getConnectID(3) == con_handle)
        {
            nano_discoverCharsOfService(3);
        }
        else if(nano_getConnectID(4) == con_handle)
        {
            nano_discoverCharsOfService(4);
        }
        else if(nano_getConnectID(5) == con_handle)
        {
            nano_discoverCharsOfService(5);
        }
        else if(nano_getConnectID(6) == con_handle)
        {
            nano_discoverCharsOfService(6);
        }
        else if(nano_getConnectID(7) == con_handle)
        { 
            nano_discoverCharsOfService(7);
        }
    }
}

static void discoveredCharsCallback(BLEStatus_t status, uint16_t con_handle, gatt_client_characteristic_t *characteristic)
{
    if(status == BLE_STATUS_OK)
    {
        Serial.println(" ");
        Serial.print("characteristic start handle: ");
        Serial.println(characteristic->start_handle, HEX);
        Serial.print("characteristic value handle: ");
        Serial.println(characteristic->value_handle, HEX);
        Serial.print("characteristic end_handle: ");
        Serial.println(characteristic->end_handle, HEX);
        Serial.print("characteristic properties: ");
        Serial.println(characteristic->properties, HEX);
        Serial.print("characteristic uuid16: ");
        Serial.println(characteristic->uuid16, HEX);
        uint8_t index;
        Serial.print("characteristic uuid128 : ");
        for(index=0; index<16; index++)
        {
            Serial.print(characteristic->uuid128[index], HEX);
            Serial.print(" ");
        }
        Serial.println(" ");

        if(nano_getConnectID(0) == con_handle)
        {
            nano_discoveredCharsResult(0,characteristic);
        }
        else if(nano_getConnectID(1) == con_handle)
        {
            nano_discoveredCharsResult(1,characteristic);
        }
        else if(nano_getConnectID(2) == con_handle)
        {
            nano_discoveredCharsResult(2,characteristic);
        }
        else if(nano_getConnectID(3) == con_handle)
        {
            nano_discoveredCharsResult(3,characteristic);
        }
        else if(nano_getConnectID(4) == con_handle)
        {
            nano_discoveredCharsResult(4,characteristic);
        }
        else if(nano_getConnectID(5) == con_handle)
        {
            nano_discoveredCharsResult(5,characteristic);
        }
        else if(nano_getConnectID(6) == con_handle)
        {
            nano_discoveredCharsResult(6,characteristic);
        }
        else if(nano_getConnectID(7) == con_handle)
        {
            nano_discoveredCharsResult(7,characteristic);
        }
    }
    else if(status == BLE_STATUS_DONE)
    {
        Serial.println("Discovered characteristic done, start to discover descriptors.");
        if(nano_getConnectID(0) == con_handle)
        {
            nano_discoverDescriptor(0);
        }
        else if(nano_getConnectID(1) == con_handle)
        {
            nano_discoverDescriptor(1);
        }
        else if(nano_getConnectID(2) == con_handle)
        {
            nano_discoverDescriptor(2);
        }
        else if(nano_getConnectID(3) == con_handle)
        {
            nano_discoverDescriptor(3);
        }
        else if(nano_getConnectID(4) == con_handle)
        {
            nano_discoverDescriptor(4);
        }
        else if(nano_getConnectID(5) == con_handle)
        {
            nano_discoverDescriptor(5);
        }
        else if(nano_getConnectID(6) == con_handle)
        { 
            nano_discoverDescriptor(6);
        }
        else if(nano_getConnectID(7) == con_handle)
        {
            nano_discoverDescriptor(7);
        }
    }
}

static void discoveredCharsDescriptorsCallback(BLEStatus_t status, uint16_t con_handle, gatt_client_characteristic_descriptor_t *descriptor)
{
    if(status == BLE_STATUS_OK)
    {
        Serial.println(" ");
        Serial.print("descriptor handle: ");
        Serial.println(descriptor->handle, HEX);
        Serial.print("descriptor uuid16: ");
        Serial.println(descriptor->uuid16, HEX);
        uint8_t index;
        Serial.print("descriptor uuid128 : ");
        for(index=0; index<16; index++)
        {
            Serial.print(descriptor->uuid128[index], HEX);
            Serial.print(" ");
        }
        Serial.println(" ");

        if(nano_getConnectID(0) == con_handle)
        {
            nano_discoverDescriptorResult(0,descriptor);
        }
        else if(nano_getConnectID(1) == con_handle)
        {
            nano_discoverDescriptorResult(1,descriptor);
        }
        else if(nano_getConnectID(2) == con_handle)
        {
            nano_discoverDescriptorResult(2,descriptor);
        }
        else if(nano_getConnectID(3) == con_handle)
        {
            nano_discoverDescriptorResult(3,descriptor);
        }
        else if(nano_getConnectID(4) == con_handle)
        {
            nano_discoverDescriptorResult(4,descriptor);
        }
        else if(nano_getConnectID(5) == con_handle)
        {
            nano_discoverDescriptorResult(5,descriptor);
        }
        else if(nano_getConnectID(6) == con_handle)
        {
            nano_discoverDescriptorResult(6,descriptor);
        }
        else if(nano_getConnectID(7) == con_handle)
        {
            nano_discoverDescriptorResult(7,descriptor);
        }
    }
    else if(status == BLE_STATUS_DONE)
    {
        Serial.println("Discovered descriptor done, open notify.");
        if(nano_getConnectID(0) == con_handle)
        {
            nano_startNotify(0);
        }
        else if(nano_getConnectID(1) == con_handle)
        {
            nano_startNotify(1);
        }
        else if(nano_getConnectID(2) == con_handle)
        {
            nano_startNotify(2);
        }
        else if(nano_getConnectID(3) == con_handle)
        {
            nano_startNotify(3);
        }
        else if(nano_getConnectID(4) == con_handle)
        {
            nano_startNotify(4);
        }
        else if(nano_getConnectID(5) == con_handle)
        {
            nano_startNotify(5);
        }
        else if(nano_getConnectID(6) == con_handle)
        {
            nano_startNotify(6);
        }
        else if(nano_getConnectID(7) == con_handle)
        {
            nano_startNotify(7);
        }
    }
}

void gattWriteCCCDCallback(BLEStatus_t status, uint16_t con_handle)
{
    if(status == BLE_STATUS_DONE)
    {
        Serial.println("gattWriteCCCDCallback done");
        if(nano_getConnectID(0) == con_handle)
        {
            peripheralDiscoveyState |= NANO_0_MASK;
        }
        else if(nano_getConnectID(1) == con_handle)
        {
            peripheralDiscoveyState |= NANO_1_MASK;
        }
        else if(nano_getConnectID(2) == con_handle)
        {
            peripheralDiscoveyState |= NANO_2_MASK;
        }
        else if(nano_getConnectID(3) == con_handle)
        {
            peripheralDiscoveyState |= NANO_3_MASK;
        }
        else if(nano_getConnectID(4) == con_handle)
        {
            peripheralDiscoveyState |= NANO_4_MASK;
        }
        else if(nano_getConnectID(5) == con_handle)
        {
            peripheralDiscoveyState |= NANO_5_MASK;
        }
        else if(nano_getConnectID(6) == con_handle)
        {
            peripheralDiscoveyState |= NANO_6_MASK;
        }
        else if(nano_getConnectID(7) == con_handle)
        {
            peripheralDiscoveyState |= NANO_7_MASK;
        }

        if(peripheralDiscoveyState != ALL_DISCOVERYED_MASK)
        {   // Discover other nano.
            if( ((ALL_DISCOVERYED_MASK & NANO_0_MASK)==NANO_0_MASK) && ((peripheralDiscoveyState & NANO_0_MASK)!=NANO_0_MASK) )
            {
                peripheralState = PERIPHERAL_NANO_0;
                nano_discoverService(0);
            }
            else if( ((ALL_DISCOVERYED_MASK & NANO_1_MASK)==NANO_1_MASK) && ((peripheralDiscoveyState & NANO_1_MASK)!=NANO_1_MASK) )
            {
                peripheralState = PERIPHERAL_NANO_1;
                nano_discoverService(1);
            }
            else if( ((ALL_DISCOVERYED_MASK & NANO_2_MASK)==NANO_2_MASK) && ((peripheralDiscoveyState & NANO_2_MASK)!=NANO_2_MASK) )
            {
                peripheralState = PERIPHERAL_NANO_2;
                nano_discoverService(2);
            }
            else if( ((ALL_DISCOVERYED_MASK & NANO_3_MASK)==NANO_3_MASK) && ((peripheralDiscoveyState & NANO_3_MASK)!=NANO_3_MASK) )
            {
                peripheralState = PERIPHERAL_NANO_3;
                nano_discoverService(3);
            }
            else if( ((ALL_DISCOVERYED_MASK & NANO_4_MASK)==NANO_4_MASK) && ((peripheralDiscoveyState & NANO_4_MASK)!=NANO_4_MASK) )
            {
                peripheralState = PERIPHERAL_NANO_4;
                nano_discoverService(4);
            }
            else if( ((ALL_DISCOVERYED_MASK & NANO_5_MASK)==NANO_5_MASK) && ((peripheralDiscoveyState & NANO_5_MASK)!=NANO_5_MASK) )
            {
                peripheralState = PERIPHERAL_NANO_5;
                nano_discoverService(5);
            }
            else if( ((ALL_DISCOVERYED_MASK & NANO_6_MASK)==NANO_6_MASK) && ((peripheralDiscoveyState & NANO_6_MASK)!=NANO_6_MASK) )
            {
                peripheralState = PERIPHERAL_NANO_6;
                nano_discoverService(6);
            }
            else if( ((ALL_DISCOVERYED_MASK & NANO_7_MASK)==NANO_7_MASK) && ((peripheralDiscoveyState & NANO_7_MASK)!=NANO_7_MASK) )
            {
                peripheralState = PERIPHERAL_NANO_7;
                nano_discoverService(7);
            }
        }
        else
        {
            // All nano are discovered.
            is_nano_ok = 1;
            // set LED to white.
            RGB.color(255, 255, 255);   
            Serial.println("All nano discover done!");
        }
    }
}

void gattNotifyUpdateCallback(BLEStatus_t status, uint16_t con_handle, uint16_t value_handle, uint8_t *value, uint16_t length)
{
    RGB.color(0, 0, 255);   
    Serial.println(" ");
    Serial.println("Notify Update value ");
    Serial.print("conn handle: ");
    Serial.println(con_handle, HEX);
    Serial.print("value handle: ");
    Serial.println(value_handle, HEX);
    uint8_t index;
    Serial.print("The value : ");
    for(index=0; index<length; index++)
    {
        Serial.print(value[index], HEX);
        Serial.print(" ");
    }
    Serial.println(" ");
    if(is_wifi_connected)
    {
        uint8_t buf[5] = {value[1], 0x00, value[2], value[3], value[4]};
        send_status(buf);
    }
    RGB.color(255, 255, 255);   
}

void gattReadCallback(BLEStatus_t status, uint16_t con_handle, uint16_t value_handle, uint8_t *value, uint16_t length)
{
    if(status == BLE_STATUS_OK)
    {  
        RGB.color(0, 0, 255);   
        Serial.println(" ");
        Serial.println("Read characteristic ok");
        Serial.print("conn handle: ");
        Serial.println(con_handle, HEX);
        Serial.print("value handle: ");
        Serial.println(value_handle, HEX);
        uint8_t index;
        Serial.print("The value : ");
        for(index=0; index<length; index++)
        {
            Serial.print(value[index], HEX);
            Serial.print(" ");
        }
        Serial.println(" ");
        if(is_wifi_connected)
        {
            uint8_t buf[5] = {value[1], 0x00, value[2], value[3], value[4]};
            send_status(buf);
        }
        RGB.color(255, 255, 255);  
    }
}

void setup()
{
    Serial.begin(115200);
    delay(3000);
                  
    // attempt to connect to Wifi network:
    Serial.print("Attempting to connect to Network named: ");
    // print the network name (SSID);
    Serial.println(ssid); 
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    WiFi.on();
    WiFi.setCredentials(ssid,password);
    WiFi.connect();
    while ( WiFi.connecting()) {
       // print dots while we wait to connect
        Serial.print(".");
        delay(300);
    }
  
    Serial.println("\nYou're connected to the network");
    Serial.println("Waiting for an ip address");
  
    IPAddress localIP = WiFi.localIP();

    while (localIP[0] == 0)
    {
        localIP = WiFi.localIP();
        Serial.println("waiting for an IP address");
        delay(1000);
    }

    Serial.println("\nIP Address obtained");
    printWifiStatus();
    Serial.println("\nStarting connection to server...");

    //ble.debugLogger(true);
    //ble.debugError(true);
    //ble.enablePacketLogger();

    ble.init();
    ble.onConnectedCallback(deviceConnectedCallback);
    ble.onDisconnectedCallback(deviceDisconnectedCallback);
    ble.onScanReportCallback(reportCallback);

    ble.onServiceDiscoveredCallback(discoveredServiceCallback);
    ble.onCharacteristicDiscoveredCallback(discoveredCharsCallback);
    ble.onDescriptorDiscoveredCallback(discoveredCharsDescriptorsCallback);
  
    ble.onGattWriteClientCharacteristicConfigCallback(gattWriteCCCDCallback);
    ble.onGattNotifyUpdateCallback(gattNotifyUpdateCallback);

    ble.onGattCharacteristicReadCallback(gattReadCallback);
    // Init all nano.
    for(uint8_t index=0; index<NANO_NUM; index++)
      nano_init(index);

    ble.setScanParams(0, 0x02A0, 0x02A0);
    ble.startScanning();
    Serial.println("Starting scanning....");
    RGB.control(true);
    // set LED to Red.
    RGB.color(255, 0, 0);   
}

void loop()
{
    if(client.connected())
    { 
        is_wifi_connected = 1;
        digitalWrite(D7, 0);
        if(client.available())
        { 
            Serial.println("Receive json...");
            
            delay(1);
            RGB.color(0, 255, 0);   
            rx_len = 0;
            memset(rx_buf, 0x00, sizeof(rx_buf));
            while(client.available()) 
            {
                rx_buf[rx_len++] = client.read();
            }  
            rx_buf[rx_len++] = '\0';         
            rx_buf[rx_len++] = EOF;            
            Serial.println(rx_buf);
            parseJson(rx_buf);
        }
    }
    else
    {   
        is_wifi_connected = 0;
        digitalWrite(D7, 1);
        client = server.available();
    }
}

