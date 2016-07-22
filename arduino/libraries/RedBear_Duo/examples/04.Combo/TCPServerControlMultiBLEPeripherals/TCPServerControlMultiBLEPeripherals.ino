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
 * Note : Remember to set the max number of peripheral at "ble_nano.h".
 *        If you want to connect to two nano, set NANO_NUM 2.
 */

#include "application.h"
#include "ble_nano.h"
#include <ArduinoJson.h>
#include "MDNS.h"

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

/*******************************************************
 *               Variable Definitions
 ******************************************************/
 // your network name also called SSID
char ssid[] = "Duo";
// your network password
char password[] = "password";

static uint8_t is_connecting_flag = 0; 
static uint8_t current_connecting_num = 0xFF;  
static uint8_t current_disconnecting_num = 0xFF;
static uint8_t  current_discovered_num = 0xFF;

static char    rx_buf[61];
static uint8_t rx_len;

static uint8_t is_nano_ok = 0;
static uint8_t is_wifi_connected = 0;

// 128bits-UUID in advertisement
static const uint8_t service_uuid[16] = { 0x66, 0x7E, 0x50, 0x17, 0x55, 0x5E, 0xE9, 0x9C, 0xE5, 0x11, 0xBC, 0xF0, 0xF8, 0x3B, 0x2D, 0x5A };

// Initialize the Ethernet client library with the IP address and port of the server
// that you want to connect to:
TCPServer server = TCPServer(8888);
TCPClient client;

MDNS mdns;

/******************************************************
 *               Function Definitions
 ******************************************************/
void mdns_init() {
  bool success = mdns.setHostname("duo");
     
  if (success) {
    success = mdns.setService("tcp", "duosample", 8888, "RedBear.8*RGB");
    Serial.println("setService");
    if (success) {
      success = mdns.begin();
      Serial.println("mdns.begin");
      if (success) {
        Spark.publish("mdns/setup", "success");
        Serial.println("mdns/setup success");
        return;
      }
    }
  }
  Spark.publish("mdns/setup", "error");
  Serial.println("mdns/setup error");
}

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

void send_number(uint8_t flag) {
  uint8_t num;
  char json_buffer[60];
  
  // If all peripherals are connected, notify the number
  if(flag) num = NANO_NUM;
  else num = 0;
  
  // Creat json string
  StaticJsonBuffer<500> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["OpCode"] = 0;
  root["ID"] = 0xF0;
  root["NUM"] = num;
  root["R"] = 0;
  root["G"] = 0;
  root["B"] = 0;
  root.printTo(json_buffer,60);
  Serial.println(json_buffer);
  // Send it
  if (client.connected()) {   
    client.println(json_buffer);
  }
}

void send_status(uint8_t *buf) {   
  char json_buffer[60];
  StaticJsonBuffer<500> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["OpCode"] = buf[0];
  root["ID"] = buf[1];
  root["NUM"] = 0xFF;
  root["R"] = buf[2];
  root["G"] = buf[3];
  root["B"] = buf[4];
  root.printTo(json_buffer,60);   
  Serial.println(json_buffer);
    
  if (client.connected()) {   
    client.println(json_buffer);
  }
  delay(100);
}

void parseJson(char *jsonString) { 
  // Parse json string
  StaticJsonBuffer<500> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(jsonString);
  if (!root.success()) {
    Serial.println("parseObject() failed");
    return;
  }
  uint8_t OpCode = root["OpCode"];
  uint8_t id = root["ID"];
  uint8_t num = root["NUM"];
  uint8_t r = root["R"];
  uint8_t g = root["G"];
  uint8_t b = root["B"];

  Serial.print("OpCode:");
  Serial.println(OpCode, HEX);
  Serial.print("id:");
  Serial.println(id, HEX);
  Serial.print("num:");
  Serial.println(num, HEX);
  Serial.print("r:");
  Serial.println(r, HEX);
  Serial.print("g:");
  Serial.println(g, HEX);    
  Serial.print("b:");
  Serial.println(b, HEX);       

  if ((OpCode == 1) && (is_nano_ok)) {   //Write command
    uint8_t buf[3]= {r, g, b};
    nano_write(id, buf, 3);
  }
  else if ((OpCode == 2) && (is_nano_ok)) {   //Read command
    nano_read(id);
  }
  else if ((OpCode == 3) && (id==0xF0)) {   //Get number of peripheral
    send_number(is_nano_ok);
  }
    
  delay(100);
  RGB.color(255, 255, 255);
}

uint32_t ble_advdata_decode(uint8_t type, uint8_t advdata_len, uint8_t *p_advdata, uint8_t *len, uint8_t *p_field_data) {
  uint8_t index=0;
  uint8_t field_length, field_type;

  while (index < advdata_len) {
    field_length = p_advdata[index];
    field_type   = p_advdata[index+1];
    if (field_type == type) {
      memcpy(p_field_data, &p_advdata[index+2], (field_length-1));
      *len = field_length - 1;
      return 0;
    }
    index += field_length + 1;
  }
  return 1;
}

void reportCallback(advertisementReport_t *report) {
  uint8_t index;

  Serial.println("reportCallback: ");
  Serial.print("The advEventType: ");
  Serial.println(report->advEventType, HEX);
  Serial.print("The peerAddrType: ");
  Serial.println(report->peerAddrType, HEX);
  Serial.print("The peerAddr: ");
  for (index = 0; index < 6; index++) {
    Serial.print(report->peerAddr[index], HEX);
    Serial.print(" ");
  }
  Serial.println(" ");

  Serial.print("The rssi: ");
  Serial.println(report->rssi, DEC);

  Serial.print("The adv_data: ");
  for (index = 0; index < report->advDataLen; index++) {
    Serial.print(report->advData[index], HEX);
    Serial.print(" ");
  }
  Serial.println(" ");

  uint8_t len;
  uint8_t adv_uuid[31];
  // Get the complete 128bits-UUID in advertisment.
  if (0x00 == ble_advdata_decode(0x07, report->advDataLen, report->advData, &len, adv_uuid)) {
    Serial.print("The service uuid: ");
    for (index = 0; index < 16; index++) {
      Serial.print(adv_uuid[index], HEX);
      Serial.print(" ");
    }
    Serial.println(" ");
    if (0x00 == memcmp(service_uuid, adv_uuid, 16)) {
      // Get a number of unconnected nano in queue.
      current_connecting_num = nano_checkUnconnected();
      Serial.println("Find nano...");
      Serial.print("Current unconnected nano : ");
      Serial.println(current_connecting_num, HEX);
      if ((is_connecting_flag == 0) && (0xFF != current_connecting_num)) {
        Serial.println("Connecting to nano...");
        ble.stopScanning();
        // Save peerAddre and connect to device.
        nano_setPeerAddr(current_connecting_num, report->peerAddr, BD_ADDR_TYPE_LE_RANDOM);
        nano_connect(current_connecting_num);
        // Make sure no other connect operation when connecting to a device.
        is_connecting_flag = 1;
      }
      else {
        Serial.println("no unconnected nano");
        ble.stopScanning();
      }
    }
  }
}

void deviceDisconnectedCallback(uint16_t handle) {
  Serial.print("Disconnected handle : ");
  Serial.println(handle,HEX);
  RGB.color(255, 0, 0);

  if (is_nano_ok) {
    is_nano_ok = 0;
    send_number(is_nano_ok);
  }
  // Get the number of nano according to the connect_handle.
  current_disconnecting_num = nano_getNumAccordingHandle(handle);
  if (0xFF != current_disconnecting_num) {
    Serial.println("PERIPHERAL_NANO disconnected.");
    //Disconnected, reset the relevant variables.
    nano_setDiscoveredState(current_disconnecting_num, NANO_DISCOVERY_IDLE);
    nano_stopNotify(current_disconnecting_num);
    nano_setConnectHandle(current_disconnecting_num, INVALID_CONN_HANDLE);
  }

  if ( (is_connecting_flag == 0) && (0xFF != nano_checkUnconnected()) ) {   // When duo is connecting to device, don't start scanning.
    Serial.println("Restart scanning.");
    ble.startScanning();
  }
}

void deviceConnectedCallback(BLEStatus_t status, uint16_t handle) {
  switch (status) {
    case BLE_STATUS_OK:
      Serial.print("Device connected : ");
      Serial.println(handle, HEX);
      if (current_connecting_num != 0xFF) {
        Serial.print("Connect to PERIPHERAL_NANO ");
        Serial.println(current_connecting_num, HEX);
        // Save conn_handle.
        nano_setConnectHandle(current_connecting_num, handle);
        current_connecting_num = 0xFF;
      }
      // Check whether all nano have been connected.
      if (0xFF == nano_checkUnconnected()) {   // All device is connected.
        Serial.println("ALL NANO Connected.");
        ble.stopScanning();

        for (uint8_t i = 0; i < NANO_NUM; i++) {
          Serial.print("Device handle : ");
          Serial.println(nano_getConnectHandle(i), HEX);   
        }
        // Start discover service.
        current_discovered_num = nano_getNumOfUndiscovered();
        if (0xFF != current_discovered_num) {
          Serial.print("Start discovered :  ");
          Serial.println(current_discovered_num, HEX);
          nano_discoverService(current_discovered_num);
        }
      }
      else {
        Serial.println("Restart scanning.");
        ble.startScanning();
      }
      is_connecting_flag = 0;
      break;
    default:
      ble.startScanning();
      break;
  } 
}

// Handle the callback event of discovered service.
static void discoveredServiceCallback(BLEStatus_t status, uint16_t con_handle, gatt_client_service_t *service) {
  uint8_t index;
  if (status == BLE_STATUS_OK) {   // Find a service.
    Serial.println(" ");
    Serial.print("Service start handle: ");
    Serial.println(service->start_group_handle, HEX);
    Serial.print("Service end handle: ");
    Serial.println(service->end_group_handle, HEX);
    Serial.print("Service uuid16: ");
    Serial.println(service->uuid16, HEX);
    Serial.print("The uuid128 : ");
    for (index = 0; index < 16; index++) {
      Serial.print(service->uuid128[index], HEX);
      Serial.print(" ");
    }
    Serial.println(" ");
        
    uint8_t num = nano_getNumAccordingHandle(con_handle);
    Serial.print("Device ID :  ");
    Serial.println(num, HEX);   
    nano_discoveredServiceResult(num, service);
  }
  else if(status == BLE_STATUS_DONE) {   // Finish.
    Serial.println("Discovered service done, start to discover chars of service.");
    uint8_t num = nano_getNumAccordingHandle(con_handle);
    Serial.print("Device ID :  ");
    Serial.println(num, HEX);   
    nano_discoverCharsOfService(num);
  }
}

// Handle the callback event of discovered characteristic.
static void discoveredCharsCallback(BLEStatus_t status, uint16_t con_handle, gatt_client_characteristic_t *characteristic) {
  uint8_t index;
  if (status == BLE_STATUS_OK) {   // Find a characteristic.
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
    Serial.print("characteristic uuid128 : ");
    for (index = 0; index < 16; index++) {
      Serial.print(characteristic->uuid128[index], HEX);
      Serial.print(" ");
    }
    Serial.println(" ");
    uint8_t num = nano_getNumAccordingHandle(con_handle);
    Serial.print("Device ID :  ");
    Serial.println(num, HEX);   
    nano_discoveredCharsResult(num, characteristic);
  }
  else if(status == BLE_STATUS_DONE) {   // Finish.
    Serial.println("Discovered characteristic done, start to discover descriptors.");
    uint8_t num = nano_getNumAccordingHandle(con_handle);
    Serial.print("Device ID :  ");
    Serial.println(num, HEX);   
    nano_discoverDescriptor(num);
  }
}

static void discoveredCharsDescriptorsCallback(BLEStatus_t status, uint16_t con_handle, gatt_client_characteristic_descriptor_t *descriptor) {
  uint8_t index;
  if (status == BLE_STATUS_OK) {   // Find a descriptor.
    Serial.println(" ");
    Serial.print("descriptor handle: ");
    Serial.println(descriptor->handle, HEX);
    Serial.print("descriptor uuid16: ");
    Serial.println(descriptor->uuid16, HEX);
    Serial.print("descriptor uuid128 : ");
    for (index = 0; index < 16; index++) {
      Serial.print(descriptor->uuid128[index], HEX);
      Serial.print(" ");
    }
    Serial.println(" ");
    uint8_t num = nano_getNumAccordingHandle(con_handle);
    Serial.print("Device ID :  ");
    Serial.println(num, HEX);   
    nano_discoverDescriptorResult(num, descriptor);
  }
  else if(status == BLE_STATUS_DONE) {   // Finish.
    Serial.println("Discovered descriptor done, open notify.");
    uint8_t num = nano_getNumAccordingHandle(con_handle);
    Serial.print("Device ID :  ");
    Serial.println(num, HEX);   
    nano_startNotify(num);
  }
}

void gattWriteCCCDCallback(BLEStatus_t status, uint16_t con_handle) {
  if (status == BLE_STATUS_DONE) {   // Open notify OK.
    Serial.println("gattWriteCCCDCallback done");
    // Finish discover,set state to NANO_DISCOVERY_FINISH.
    uint8_t num = nano_getNumAccordingHandle(con_handle);
    Serial.print("Device ID :  ");
    Serial.println(num, HEX);   
    nano_setDiscoveredState(num, NANO_DISCOVERY_FINISH);
        
    current_discovered_num = nano_getNumOfUndiscovered();
    Serial.print("Next device ID :  ");
    Serial.println(current_discovered_num, HEX);   
    if (0xFF != current_discovered_num) {   // Start discover other device.
      nano_discoverService(current_discovered_num);
    }
    else {
      // All peripherals are discovered.
      is_nano_ok = 1;
      send_number(is_nano_ok);
      // set LED to white.
      RGB.color(255, 255, 255);   
      Serial.println("All nano discover done!");
    }
  }
  else {
    Serial.println("gattWriteCCCDCallback fail");
    Serial.print("con_handle: ");
    Serial.println(con_handle, HEX);
  }
}

//Handler the notify event from device.
void gattNotifyUpdateCallback(BLEStatus_t status, uint16_t con_handle, uint16_t value_handle, uint8_t *value, uint16_t length) {
  uint8_t index;
  Serial.println(" ");
  Serial.println("Notify Update value ");
  Serial.print("conn handle: ");
  Serial.println(con_handle, HEX);
  Serial.print("value handle: ");
  Serial.println(value_handle, HEX);
  Serial.print("device id: ");
  Serial.println(nano_getNumAccordingHandle(con_handle), HEX);    
    
  Serial.print("The value : ");
  for (index = 0; index < length; index++) {
    Serial.print(value[index], HEX);
    Serial.print(" ");
  }
  Serial.println(" ");
  if (is_wifi_connected && is_nano_ok) { 
    RGB.color(0, 0, 255);   
    Serial.println("Send!");
    // The buf is { "OpCode","ID", "R", "G", "B"};
    uint8_t buf[5] = {0x00, nano_getNumAccordingHandle(con_handle), value[0], value[1], value[2]};
    send_status(buf);
    RGB.color(255, 255, 255);
  }
}

void gattReadCallback(BLEStatus_t status, uint16_t con_handle, uint16_t value_handle, uint8_t *value, uint16_t length) {
  uint8_t index;
  if (status == BLE_STATUS_OK) {  
    Serial.println(" ");
    Serial.println("Read characteristic ok");
    Serial.print("conn handle: ");
    Serial.println(con_handle, HEX);
    Serial.print("value handle: ");
    Serial.println(value_handle, HEX);
    Serial.print("device id: ");
    Serial.println(nano_getNumAccordingHandle(con_handle), HEX);  
        
    Serial.print("The value : ");
    for (index = 0; index < length; index++) {
      Serial.print(value[index], HEX);
      Serial.print(" ");
    }
    Serial.println(" ");
    if (is_wifi_connected && is_nano_ok) {
      RGB.color(0, 0, 255);   
      Serial.println("Send!");
      uint8_t buf[5] = {0x00, nano_getNumAccordingHandle(con_handle), value[0], value[1], value[2]};
      send_status(buf);
      RGB.color(255, 255, 255);
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(3000);
  pinMode(D7, OUTPUT);
  Serial.println("Arduino sketch started.\n");
    
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
  while (localIP[0] == 0) {
    localIP = WiFi.localIP();
    Serial.println("waiting for an IP address");
    delay(1000);
  }

  Serial.println("\nIP Address obtained");
  printWifiStatus();
  Serial.println("\nStarting connection to server...");   
    
  mdns_init();      
             
  //ble.debugLogger(true);
  //ble.debugError(true);
  //ble.enablePacketLogger();

  // Initialize ble_stack.
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
  for (uint8_t index = 0; index < NANO_NUM; index++)
    nano_init(index);

  ble.setScanParams(0, 0x02A0, 0x02A0);
  ble.startScanning();
  Serial.println("Starting scanning....");
  RGB.control(true);
  RGB.color(255, 0, 0);
}

void loop() {
  mdns.processQueries();
  if (client.connected()) { 
    is_wifi_connected = 1;
    digitalWrite(D7, 1);
    if (client.available()) { 
      Serial.println("Receive json...");
      delay(1);
      rx_len = 0;
      memset(rx_buf, 0x00, sizeof(rx_buf));
      while (client.available()) {
        rx_buf[rx_len++] = client.read();
        if(rx_len>=60) rx_len = 60;
      }            
      Serial.println(rx_buf);
      if (is_nano_ok) {   
        RGB.color(0, 255, 0);
        parseJson(rx_buf);
      }
    }
  }
  else {   
    is_wifi_connected = 0;
    digitalWrite(D7, 0);
    client = server.available();
  }
}

