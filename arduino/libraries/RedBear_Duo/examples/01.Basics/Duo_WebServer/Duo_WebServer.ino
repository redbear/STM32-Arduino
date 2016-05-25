#include "MDNS.h"
/******************************************************
 *                      Macros
 ******************************************************/
#if defined(ARDUINO) 
SYSTEM_MODE(MANUAL);//do not connect to cloud
#else
SYSTEM_MODE(AUTOMATIC);//connect to cloud
#endif

#define DEVICE_ID_ADDR (0x1FFF7A10)
#define DEVICE_ID_LEN  12

// BLE peripheral preferred connection parameters
#define MIN_CONN_INTERVAL          0x0028 // 50ms. Minimum connection interval = MIN_CONN_INTERVAL * 1.25 ms, where MIN_CONN_INTERVAL ranges from 0x0006 to 0x0C80.
#define MAX_CONN_INTERVAL          0x0190 // 500ms. Maximum connection interval = MAX_CONN_INTERVAL * 1.25 ms,  where MAX_CONN_INTERVAL ranges from 0x0006 to 0x0C80.
#define SLAVE_LATENCY              0x0000 // No slave latency. The SLAVE_LATENCY ranges from 0x0000 to 0x03E8.
#define CONN_SUPERVISION_TIMEOUT   0x03E8 // 10s. Connection supervision timeout = CONN_SUPERVISION_TIMEOUT * 10 ms, where CONN_SUPERVISION_TIMEOUT ranges from 0x000A to 0x0C80.

// Learn about appearance: http://developer.bluetooth.org/gatt/characteristics/Pages/CharacteristicViewer.aspx?u=org.bluetooth.characteristic.gap.appearance.xml
#define BLE_PERIPHERAL_APPEARANCE  BLE_APPEARANCE_UNKNOWN

#define CHARACTERISTIC1_MAX_LEN    3

#define BLE_DEVICE_NAME            "Duo_WebServer"

/******************************************************
 *               Variable Definitions
 ******************************************************/
static uint8_t service1_uuid[16]    = { 0x71,0x3d,0x00,0x00,0x50,0x3e,0x4c,0x75,0xba,0x94,0x31,0x48,0xf1,0x8d,0x94,0x1e };
static uint8_t service1_tx_uuid[16] = { 0x71,0x3d,0x00,0x03,0x50,0x3e,0x4c,0x75,0xba,0x94,0x31,0x48,0xf1,0x8d,0x94,0x1e };

static uint8_t  appearance[2] = { 
  LOW_BYTE(BLE_PERIPHERAL_APPEARANCE), HIGH_BYTE(BLE_PERIPHERAL_APPEARANCE) 
};

static uint8_t  change[4] = { 
  0x00, 0x00, 0xFF, 0xFF
};

// BLE connection params
// Connection interval 
//  Range: 0x0006 to 0x0C80
//  Time = N * 1.25 msec
//  Time Range: 7.5 msec to 4000 msec.
//Slave latency 
//  Range: 0x0000 to 0x01F3
//Connection supervision timeout 
//  Range: 0x000A to 0x0C80
//  Time = N * 10 msec
//  Time Range: 100 msec to 32 seconds
static uint8_t  conn_param[8] = {
  LOW_BYTE(MIN_CONN_INTERVAL), HIGH_BYTE(MIN_CONN_INTERVAL), 
  LOW_BYTE(MAX_CONN_INTERVAL), HIGH_BYTE(MAX_CONN_INTERVAL), 
  LOW_BYTE(SLAVE_LATENCY), HIGH_BYTE(SLAVE_LATENCY), 
  LOW_BYTE(CONN_SUPERVISION_TIMEOUT), HIGH_BYTE(CONN_SUPERVISION_TIMEOUT)
};

// BLE peripheral advertising parameters
// Note  advertising_interval_min ([0x0020,0x4000], default: 0x0800, unit: 0.625 msec)
//       advertising_interval_max ([0x0020,0x4000], default: 0x0800, unit: 0.625 msec)
//       advertising_type (enum from 0: BLE_GAP_ADV_TYPE_ADV_IND, BLE_GAP_ADV_TYPE_ADV_DIRECT_IND, BLE_GAP_ADV_TYPE_ADV_SCAN_IND, BLE_GAP_ADV_TYPE_ADV_NONCONN_IND)
//       own_address_type (enum from 0: BLE_GAP_ADDR_TYPE_PUBLIC, BLE_GAP_ADDR_TYPE_RANDOM)
//       advertising_channel_map (flags: BLE_GAP_ADV_CHANNEL_MAP_37, BLE_GAP_ADV_CHANNEL_MAP_38, BLE_GAP_ADV_CHANNEL_MAP_39, BLE_GAP_ADV_CHANNEL_MAP_ALL)
//       filter policies (enum from 0: BLE_GAP_ADV_FP_ANY, BLE_GAP_ADV_FP_FILTER_SCANREQ, BLE_GAP_ADV_FP_FILTER_CONNREQ, BLE_GAP_ADV_FP_FILTER_BOTH)
// Note  If the advertising_type is set to BLE_GAP_ADV_TYPE_ADV_SCAN_IND or BLE_GAP_ADV_TYPE_ADV_NONCONN_IND,advertising_interval_min and advertising_interval_max shal not be set to less than 0x00A0.
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

  0x11,
  BLE_GAP_AD_TYPE_128BIT_SERVICE_UUID_COMPLETE,
  0x1e,0x94,0x8d,0xf1,0x48,0x31,0x94,0xba,0x75,0x4c,0x3e,0x50,0x00,0x00,0x3d,0x71
};

// BLE peripheral scan respond data
static uint8_t scan_response[]={
  0x08,
  BLE_GAP_AD_TYPE_COMPLETE_LOCAL_NAME,
  'B','i','s','c','u','i','t',
};

static uint16_t character1_handle = 0x0000;
static uint8_t characteristic1_data[CHARACTERISTIC1_MAX_LEN] = { 0x01 };

TCPServer server = TCPServer(80);
TCPClient client;
MDNS mdns;

int led1 = D7;

 /******************************************************
 *               Function Definitions
 ******************************************************/
boolean endsWith(char* inString, char* compString);
void mdns_init();
void printWifiStatus();

/* a way to check if one array ends with another array */
boolean endsWith(char* inString, char* compString) {
  int compLength = strlen(compString);
  int strLength = strlen(inString);

  //compare the last "compLength" values of the inString
  int i;
  for (i = 0; i < compLength; i++) {
    char a = inString[(strLength - 1) - i];
    char b = compString[(compLength - 1) - i];
    if (a != b) {
      return false;
    }
  }
  return true;
}

void mdns_init() {
  bool success = mdns.setHostname("duo");
  if (success) {
    success = mdns.setService("tcp", "duosample", 80, "Duo Example Web");
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

/*
 * BLE peripheral callbacks
 */
void deviceConnectedCallback(BLEStatus_t status, uint16_t handle) {
  switch (status){
    case BLE_STATUS_OK:
      Serial.println("Device connected!");
      break;
    default:
      break;
  }
}

void deviceDisconnectedCallback(uint16_t handle) {
  Serial.println("Disconnected.");
}

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
        digitalWrite(led1, HIGH);
      else
        digitalWrite(led1, LOW);
    }   
  }
  return 0;
}


void setup() {
  pinMode(led1, OUTPUT);

  Serial.begin(115200);
  delay(5000);

  Serial.println("Arduino sketch started.\n");
    
  local_name_t local_name;
  HAL_Local_Name(&local_name);

  Serial.print("Local Name: ");
  for( uint8_t i = 0; i < local_name.length; i++) {
    Serial.write(local_name.value[i]);
  }
  Serial.println("\n");

  uint8_t dev_id[DEVICE_ID_LEN];
  memcpy(dev_id, (char*)DEVICE_ID_ADDR, DEVICE_ID_LEN);

  Serial.print("Device ID: ");
  for (uint8_t i = 0; i < DEVICE_ID_LEN; i++) {
    uint8_t c;
    c = (dev_id[i] >> 4) + 48;
    if (c > 57) c += 39;
    Serial.write(c);
    c = (dev_id[i] & 0x0F) + 48;
    if (c > 57) c += 39;
    Serial.write(c);
  }
  Serial.println("\n");
    
  Serial.println("Note: If your Duo hasn't stored a valid WiFi profile, it will enter the listening mode for provisioning first.\n");

  WiFi.on();
  WiFi.connect();

  Serial.println("Waiting for an IP address...\n");
  while (!WiFi.ready()) {
    delay(1000);
  }
  // Wait IP address to be updated.
  IPAddress localIP = WiFi.localIP();
  while (localIP[0] == 0) {
    localIP = WiFi.localIP();
    delay(1000);
  }

  // you're connected now, so print out the status:
  printWifiStatus();

  Serial.println("Make sure your smart device acting as web client is connecting to the same AP as Duo.");
  Serial.println("Then open the web browser on your smart device and enter the Duo's web server IP address.");
  Serial.println(" ");

  server.begin();
  mdns_init();

  //ble.debugLogger(true);
  ble.init(); // Must be called before other BLE functions invoked.

  // Register BLE callbacks
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

  // Set BLE advertising parameters
  ble.setAdvertisementParams(&adv_params);
  // Set BLE advertising and scan respond data
  ble.setAdvertisementData(sizeof(adv_data), adv_data);
  ble.setScanResponseData(sizeof(scan_response), scan_response);

  // BLE peripheral starts advertising now.
  ble.startAdvertising();
  Serial.println("BLE start advertising.");
}

void loop() {
  mdns.processQueries();
  int i = 0;

  if (client.connected()) {
    Serial.println("new client");           // print a message out the serial port

    char buffer[150] = {0};                 // make a buffer to hold incoming data
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (strlen(buffer) == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

            // the content of the HTTP response follows the header:
            client.println("<html><head><title>RedBear Duo WiFi Web Server</title></head><body align=center>");
            client.println("<h1 align=center><font color=\"red\">Welcome to the RedBear Duo WiFi Web Server</font></h1>");
            client.print("LED <button onclick=\"location.href='/H'\">HIGH</button>");
            client.println(" <button onclick=\"location.href='/L'\">LOW</button><br>");

            // The HTTP response ends with another blank line:
            client.println();
            // break out of the while loop:
            break;
          }
          else {      // if you got a newline, then clear the buffer:
            memset(buffer, 0, 150);
            i = 0;
          }
        }
        else if (c != '\r') {    // if you got anything else but a carriage return character,
          buffer[i++] = c;      // add it to the end of the currentLine
        }

        // Check to see if the client request was "GET /H" or "GET /L":
        if (endsWith(buffer, "GET /H")) {
          digitalWrite(led1, HIGH);               // GET /H turns the LED on
        }
        if (endsWith(buffer, "GET /L")) {
          digitalWrite(led1, LOW);                // GET /L turns the LED off
        }
      }
    }
    // close the connection:
    client.stop();
    Serial.println("client disonnected");
  }
  else {
    client = server.available();
  }
}

