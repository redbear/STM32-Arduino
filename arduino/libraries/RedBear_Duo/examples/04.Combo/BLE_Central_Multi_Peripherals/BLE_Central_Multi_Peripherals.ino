
#include "application.h"
#include "ble_nano.h"
#include "aJSON.h"

#if defined(ARDUINO) 
SYSTEM_MODE(MANUAL);//do not connect to cloud
#else
SYSTEM_MODE(AUTOMATIC);//connect to cloud
#endif

// your network name also called SSID
char ssid[] = "Duo";
// your network password
char password[] = "password";

// Initialize the Ethernet client library
// with the IP address and port of the server
// that you want to connect to:
TCPServer server = TCPServer(8888);
TCPClient client;

/*******************************************************
 *               Variable Definitions
 ******************************************************/
static uint8_t        is_connecting_flag = 0; 
static uint8_t        current_connecting_num = 0xFF;  
static uint8_t        current_disconnecting_num = 0xFF;
static uint8_t        current_discovered_num = 0xFF;

static char rx_buf[100];
static uint8_t rx_len;

// Timer task for setting RGB.
static btstack_timer_source_t rgb_config;

static uint8_t is_nano_ok = 0;
static uint8_t is_wifi_connected = 0;
// 128bits-UUID in advertisement
static const uint8_t service_uuid[16] = {0x66, 0x7E, 0x50, 0x17, 0x55, 0x5E, 0xE9, 0x9C, 0xE5, 0x11, 0xBC, 0xF0, 0xF8, 0x3B, 0x2D, 0x5A};
/******************************************************
 *               Function Definitions
 ******************************************************/
void rgb_config_handler(btstack_timer_source_t *ts)
{
    RGB.color(255, 255, 255);   
}

void printWifiStatus() 
{
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
    // Create JSON stream.
    aJson.addItemToObject(temp, "ID", aJson.createItem(buf[0]));
    aJson.addItemToObject(temp, "OpCode", aJson.createItem(buf[1]));
    aJson.addItemToObject(temp, "R", aJson.createItem(buf[2]));
    aJson.addItemToObject(temp, "G", aJson.createItem(buf[3]));
    aJson.addItemToObject(temp, "B", aJson.createItem(buf[4]));
    aJson.addItemToObject(root, "RGB", temp);
    
    const char *p = aJson.print(root);

    aJson.deleteItem(root);
    aJson.deleteItem(temp);
    // Send JSON stream to client.
    Serial.println(p);
    client.println(p);
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
            // Parse JSON stream.
            if( (id!=NULL) && (OpCode!=NULL) && (r!=NULL) && (g!=NULL) && (b!=NULL) )
            {
                if(OpCode->valueint == 1)
                {   // Write command.
                    uint8_t buf[5] = {0xA5, 0xFF, r->valueint, g->valueint, b->valueint};
                    // Check whether all nano is connected and discovered.
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
    uint8_t adv_uuid[31];
    // Get the complete 128bits-UUID in advertisment.
    if(0x00 == ble_advdata_decode(0x07, report->advDataLen, report->advData, &len, adv_uuid))
    {
        Serial.print("The service uuid: ");
        for(index=0; index<16; index++)
        {
            Serial.print(adv_uuid[index], HEX);
            Serial.print(" ");
        }
        Serial.println(" ");
        if(0x00 == memcmp(service_uuid, adv_uuid, 16))
        {
            // Get a number of unconnected nano in queue.
            current_connecting_num = nano_checkUnconnected();
            Serial.println("Find nano...");
            Serial.print("Current unconnected nano : ");
            Serial.println(current_connecting_num, HEX);
            if( (is_connecting_flag == 0) && (0xFF != current_connecting_num))
            {
                Serial.println("Connecting to nano...");
                ble.stopScanning();
                // Save peerAddre and connect to device.
                nano_setPeerAddr(current_connecting_num, report->peerAddr, BD_ADDR_TYPE_LE_RANDOM);
                nano_connect(current_connecting_num);
                // Make sure no other connect operation when connecting to a device.
                is_connecting_flag = 1;
            }
            else
            {
                Serial.println("no unconnected nano");
                ble.stopScanning();
            }
        }
    }
}

void deviceDisconnectedCallback(uint16_t handle){
    Serial.print("Disconnected handle : ");
    Serial.println(handle,HEX);
    RGB.color(255, 0, 0);
    
    is_nano_ok = 0;
    // Get the number of nano according to the connect_handle.
    current_disconnecting_num = nano_getNumAccordingHandle(handle);
    if(0xFF != current_disconnecting_num)
    {
        Serial.println("PERIPHERAL_NANO disconnected.");
        //Disconnected, reset the relevant variables.
        nano_setDiscoveredState(current_disconnecting_num, NANO_DISCOVERY_IDLE);
        nano_stopNotify(current_disconnecting_num);
        nano_setConnectHandle(current_disconnecting_num, INVALID_CONN_HANDLE);
    }

    if( (is_connecting_flag == 0) && (0xFF != nano_checkUnconnected()) )
    {   // When duo is connecting to device, don't start scanning.
        Serial.println("Restart scanning.");
        ble.startScanning();
    }
}

void deviceConnectedCallback(BLEStatus_t status, uint16_t handle) {
    switch (status){
        case BLE_STATUS_OK:
            Serial.print("Device connected : ");
            Serial.println(handle, HEX);
            if(current_connecting_num != 0xFF)
            {
                Serial.println("Connect to PERIPHERAL_NANO.");
                // Save conn_handle.
                nano_setConnectHandle(current_connecting_num, handle);
            }
            // Check whether all nano have been connected.
            if(0xFF == nano_checkUnconnected())
            {   // All device is connected.
                Serial.println("ALL NANO Connected.");
                ble.stopScanning();

                for(uint8_t i=0; i<NANO_NUM; i++)
                {
                    Serial.print("Device handle : ");
                    Serial.println(nano_getConnectHandle(i), HEX);   
                }
                // Start discover service.
                current_discovered_num = nano_getNumOfUndiscovered();
                if(0xFF != current_discovered_num)
                {
                    nano_discoverService(current_discovered_num);
                }
            }
            else
            {
                Serial.println("Restart scanning.");
                ble.startScanning();
            }
            break;
        default:
            ble.startScanning();
            break;
    }
    is_connecting_flag = 0;
}

// Handle the callback event of discovered service.
static void discoveredServiceCallback(BLEStatus_t status, uint16_t con_handle, gatt_client_service_t *service)
{
    if(status == BLE_STATUS_OK)
    {   // Find a service.
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
           
        nano_discoveredServiceResult(nano_getNumAccordingHandle(con_handle), service);
    }
    else if(status == BLE_STATUS_DONE)
    {   // Finish.
        Serial.println("Discovered service done, start to discover chars of service.");
        nano_discoverCharsOfService(nano_getNumAccordingHandle(con_handle));
    }
}

// Handle the callback event of discovered characteristic.
static void discoveredCharsCallback(BLEStatus_t status, uint16_t con_handle, gatt_client_characteristic_t *characteristic)
{
    if(status == BLE_STATUS_OK)
    {   // Find a characteristic.
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

        nano_discoveredCharsResult(nano_getNumAccordingHandle(con_handle), characteristic);
    }
    else if(status == BLE_STATUS_DONE)
    {   // Finish.
        Serial.println("Discovered characteristic done, start to discover descriptors.");
        nano_discoverDescriptor(nano_getNumAccordingHandle(con_handle));
    }
}

static void discoveredCharsDescriptorsCallback(BLEStatus_t status, uint16_t con_handle, gatt_client_characteristic_descriptor_t *descriptor)
{
    if(status == BLE_STATUS_OK)
    {   // Find a descriptor.
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
        nano_discoverDescriptorResult(nano_getNumAccordingHandle(con_handle), descriptor);
    }
    else if(status == BLE_STATUS_DONE)
    {   // Finish.
        Serial.println("Discovered descriptor done, open notify.");
        nano_startNotify(nano_getNumAccordingHandle(con_handle));
    }
}

void gattWriteCCCDCallback(BLEStatus_t status, uint16_t con_handle)
{
    if(status == BLE_STATUS_DONE)
    {   // Open notify OK.
        Serial.println("gattWriteCCCDCallback done");
        // Finish discover,set state to NANO_DISCOVERY_FINISH.
        nano_setDiscoveredState(nano_getNumAccordingHandle(con_handle), NANO_DISCOVERY_FINISH);
        current_discovered_num = nano_getNumOfUndiscovered();
        if(0xFF != current_discovered_num)
        {   // Start discover other device.
            nano_discoverService(current_discovered_num);
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

//Handler the notify event from device.
void gattNotifyUpdateCallback(BLEStatus_t status, uint16_t con_handle, uint16_t value_handle, uint8_t *value, uint16_t length)
{
    RGB.color(0, 0, 255);   
    ble.setTimer(&rgb_config, 200);
    ble.addTimer(&rgb_config);  
    Serial.println(" ");
    Serial.println("Notify Update value ");
    Serial.print("conn handle: ");
    Serial.println(con_handle, HEX);
    Serial.print("value handle: ");
    Serial.println(value_handle, HEX);
    Serial.print("device id: ");
    Serial.println(nano_getNumAccordingHandle(con_handle), HEX);    
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
        // The buf is {"ID", "OpCode", "R", "G", "B"};
        uint8_t buf[5] = {nano_getNumAccordingHandle(con_handle), 0x00, value[2], value[3], value[4]};
        send_status(buf);
    }
}

void gattReadCallback(BLEStatus_t status, uint16_t con_handle, uint16_t value_handle, uint8_t *value, uint16_t length)
{
    if(status == BLE_STATUS_OK)
    {  
        RGB.color(0, 0, 255);   
        ble.setTimer(&rgb_config, 200);
        ble.addTimer(&rgb_config);
        Serial.println(" ");
        Serial.println("Read characteristic ok");
        Serial.print("conn handle: ");
        Serial.println(con_handle, HEX);
        Serial.print("value handle: ");
        Serial.println(value_handle, HEX);
        Serial.print("device id: ");
        Serial.println(nano_getNumAccordingHandle(con_handle), HEX);  
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
            uint8_t buf[5] = {nano_getNumAccordingHandle(con_handle), 0x00, value[2], value[3], value[4]};
            send_status(buf);
        }
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
    // Set LED to Red.
    RGB.color(255, 0, 0);   
    // Regist rgb process handler.
    rgb_config.process = &rgb_config_handler;
}

void loop()
{
    if(client.connected())
    { 
        is_wifi_connected = 1;
        digitalWrite(D7, 1);
        if(client.available())
        { 
            Serial.println("Receive json...");
            RGB.color(0, 255, 0);   
            // Set timer.
            ble.setTimer(&rgb_config, 200);
            ble.addTimer(&rgb_config);
            
            delay(1);
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
        digitalWrite(D7, 0);
        client = server.available();
    }
}

