#include "MDNS.h"
/******************************************************
 *                      Macros
 ******************************************************/
#define DEVICE_ID_ADDR (0x1FFF7A10)
#define DEVICE_ID_LEN  12

#if defined(ARDUINO) 
SYSTEM_MODE(MANUAL);//do not connect to cloud
#else
SYSTEM_MODE(AUTOMATIC);//connect to cloud
#endif

#define CHARACTERISTIC1_MAX_LEN    3

#define DEVICE_NAME                "Duo_WebServer"

TCPServer server = TCPServer(80);
TCPClient client;
MDNS mdns;

/******************************************************
 *               Variable Definitions
 ******************************************************/
static uint8_t service1_uuid[16]       ={0x71,0x3d,0x00,0x00,0x50,0x3e,0x4c,0x75,0xba,0x94,0x31,0x48,0xf1,0x8d,0x94,0x1e};
static uint8_t service1_tx_uuid[16]    ={0x71,0x3d,0x00,0x03,0x50,0x3e,0x4c,0x75,0xba,0x94,0x31,0x48,0xf1,0x8d,0x94,0x1e};

static uint8_t  appearance[2]    = {0x00, 0x02};
static uint8_t  change[2]        = {0x00, 0x00};
static uint8_t  conn_param[8]    = {0x28, 0x00, 0x90, 0x01, 0x00, 0x00, 0x90, 0x01};

static uint16_t character1_handle = 0x0000;

static uint8_t characteristic1_data[CHARACTERISTIC1_MAX_LEN]={0x01};

static advParams_t adv_params;
static uint8_t adv_data[]={0x02,0x01,0x06, 0x11,0x07,0x1e,0x94,0x8d,0xf1,0x48,0x31,0x94,0xba,0x75,0x4c,0x3e,0x50,0x00,0x00,0x3d,0x71};


int led1 = D7;


 /******************************************************
 *               Function Definitions
 ******************************************************/

boolean endsWith(char* inString, char* compString);
void mdns_init();
void printWifiStatus();


//
//a way to check if one array ends with another array
//
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

void mdns_init()
{
    bool success = mdns.setHostname("duo");
     
    if (success) {
        success = mdns.setService("tcp", "duosample", 80, "Duo Example Web");
        Serial.println("setService");
    }

    if (success) {
        success = mdns.begin();
        Serial.println("mdns.begin");
    }
    
    if (success) {
        Spark.publish("mdns/setup", "success");
        Serial.println("mdns/setup success");
    } else {
        Spark.publish("mdns/setup", "error");
        Serial.println("mdns/setup error");
    }
}


 
void deviceConnectedCallback(BLEStatus_t status, uint16_t handle) {
    switch (status){
        case BLE_STATUS_OK:
            Serial.println("Device connected!");
            break;
        default:
            break;
    }
}

void deviceDisconnectedCallback(uint16_t handle){
    Serial.println("Disconnected.");
}

int gattWriteCallback(uint16_t value_handle, uint8_t *buffer, uint16_t size)
{
    Serial.print("Write value handler: ");
    Serial.println(value_handle, HEX);

    if(character1_handle == value_handle)
    {
        memcpy(characteristic1_data, buffer, CHARACTERISTIC1_MAX_LEN);
        Serial.print("Characteristic1 write value: ");
        for(uint8_t index=0; index<CHARACTERISTIC1_MAX_LEN; index++)
        {
            Serial.print(characteristic1_data[index], HEX);
            Serial.print(" ");
        }
        Serial.println(" ");
        //Process the data
        if (characteristic1_data[0] == 0x01)  // Command is to control digital out pin
        {
            if (characteristic1_data[1] == 0x01)
                digitalWrite(led1, HIGH);
            else
                digitalWrite(led1, LOW);
        }
        
    }
    return 0;
}


void setup()
{
    pinMode(led1, OUTPUT);

    Serial.begin(115200);
    delay(5000);

    Serial.println("Arduino sketch started.\n");
    
    local_name_t local_name;
    HAL_Local_Name(&local_name);

    Serial.print("Local Name: ");
    for(uint8_t i=0; i<local_name.length; i++)
    {
      Serial.write(local_name.value[i]);
    }
    Serial.println("\n");

    uint8_t dev_id[DEVICE_ID_LEN];
    memcpy(dev_id, (char*)DEVICE_ID_ADDR, DEVICE_ID_LEN);

    Serial.print("Device ID: ");
    for(uint8_t i=0; i<DEVICE_ID_LEN; i++)
    {
      uint8_t c;
      c = (dev_id[i]>>4) + 48;
      if(c>57) c += 39;
      Serial.write(c);
      c = (dev_id[i]&0x0F) + 48;
      if(c>57) c += 39;
      Serial.write(c);
    }
    Serial.println("\n");
    
    Serial.println("Note: If your Duo hasn't stored a valid WiFi profile, it will enter the listening mode for provisioning first.\n");

    WiFi.on();
    WiFi.connect();

    Serial.println("Waiting for an IP address...\n");
    while (!WiFi.ready())
    {
        delay(1000);
    }

    // Delay more than 1s to let the core update the IP configurations.
    delay(2000);

    IPAddress localIP = WiFi.localIP();

    Serial.print("Duo's web server IP Address: ");
    Serial.print(localIP[0], DEC);
    Serial.print(".");
    Serial.print(localIP[1], DEC);
    Serial.print(".");
    Serial.print(localIP[2], DEC);
    Serial.print(".");
    Serial.print(localIP[3], DEC);
    Serial.println(" ");
    Serial.println(" ");

    Serial.println("Make sure your smart device acting as web client is connecting to the same AP as Duo.");
    Serial.println("Then open the web browser on your smart device and enter the Duo's web server IP address.");
    Serial.println(" ");

    server.begin();
    mdns_init();

    // you're connected now, so print out the status:
    printWifiStatus();
    //ble.debugLogger(true);
    ble.init();

    ble.onConnectedCallback(deviceConnectedCallback);
    ble.onDisconnectedCallback(deviceDisconnectedCallback);
    ble.onDataWriteCallback(gattWriteCallback);

    ble.addService(0x1800);
    ble.addCharacteristic(0x2A00, ATT_PROPERTY_READ|ATT_PROPERTY_WRITE, (uint8_t*)DEVICE_NAME, sizeof(DEVICE_NAME));
    ble.addCharacteristic(0x2A01, ATT_PROPERTY_READ, appearance, sizeof(appearance));
    ble.addCharacteristic(0x2A04, ATT_PROPERTY_READ, conn_param, sizeof(conn_param));
    ble.addService(0x1801);
    ble.addCharacteristic(0x2A05, ATT_PROPERTY_INDICATE, change, sizeof(change));

    ble.addService(service1_uuid);
    character1_handle = ble.addCharacteristicDynamic(service1_tx_uuid, ATT_PROPERTY_NOTIFY|ATT_PROPERTY_WRITE|ATT_PROPERTY_WRITE_WITHOUT_RESPONSE, characteristic1_data, CHARACTERISTIC1_MAX_LEN);
   
    adv_params.adv_int_min = 0x0030;
    adv_params.adv_int_max = 0x0030;
    adv_params.adv_type    = 0;
    adv_params.dir_addr_type = 0;
    memset(adv_params.dir_addr,0,6);
    adv_params.channel_map = 0x07;
    adv_params.filter_policy = 0x00;
    
    ble.setAdvParams(&adv_params);
    
    ble.setAdvData(sizeof(adv_data), adv_data);

    ble.startAdvertising();

    Serial.println("BLE start advertising.");

}

void loop()
{
    mdns.processQueries();
    int i = 0;

    if (client.connected())
    {
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
    else
    {
        client = server.available();
    }
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

