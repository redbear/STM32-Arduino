/*

 This example demonstrates using the cc3200 as a wifi client (TCP)
 The client will connect to the TCP server.
 
 Write something in the serial console to send it to the server.
 Anything the server sends back will be printed in the serial console

 modified for Duo Board
 
 created 3 July 2014
 by Noah Luskey
 modified 1 DEC 2015
 by Jackson Lv
 */
 // your network name also called SSID
char ssid[] = "duo";
// your network password
char password[] = "password";

#if defined(ARDUINO) 
SYSTEM_MODE(MANUAL);//do not connect to cloud
#else
SYSTEM_MODE(AUTOMATIC);//connect to cloud
#endif

uint16_t port = 9999;     // port number of the server
IPAddress server(192, 168, 0, 0);   // IP Address of the server
TCPClient client;

void printWifiStatus();

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(115200);

  // attempt to connect to Wifi network:
  Serial.print("Attempting to connect to Network named: ");
  // print the network name (SSID);
  Serial.println(ssid); 
  // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
  WiFi.on();
  WiFi.setCredentials(ssid, password);
  WiFi.connect();

  while (WiFi.connecting()) {
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
  
  // you're connected now, so print out the status  
  printWifiStatus();
  
  // attempt to connect to the server
  Serial.println("Attempting to connect to server");

  uint8_t tries = 0;
  while (client.connect(server, port) == false) {
    Serial.print(".");
    if (tries++ > 100) {
      Serial.println("\nThe server isn't responding");
      while(1);
    }
    delay(100);
  }
  
  //we've connected to the server by this point
  Serial.println("\nConnected to the server!");
}

void loop() {  
  if (Serial.available()) {
    //read the serial command into a buffer
    char buffer[255] = {0};
    Serial.readBytes(buffer, Serial.available());
    //send the serial command to the server
    client.println(buffer);
    Serial.print("Sent: ");
    Serial.println(buffer);
  }
  
  if (client.available()) {
    char buffer[255] = {0};
    client.read((uint8_t*)buffer, client.available());
    Serial.print("Received: ");
    Serial.println(buffer);
  }
  
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("Network Name: ");
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
