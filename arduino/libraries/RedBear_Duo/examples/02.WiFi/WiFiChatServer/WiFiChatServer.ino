/*
 Chat  Server

 A simple server that distributes any incoming messages to all
 connected clients.  To use telnet to  your device's IP address and type.
 You can see the client's input in the serial monitor as well.

 This example is written for a network using WPA encryption. For
 WEP or WPA, change the Wifi.begin() call accordingly.


 Circuit:
 * Origin for CC3200 LaunchPad or 
   F5529/TivaC LaunchPad with CC3000/CC3100 BoosterPack
 * Modified for DUO Board   
 
 created 18 Dec 2009
 by David A. Mellis
 modified 31 May 2012
 by Tom Igoe
 modified 1 DEC 2015
 by Jackson Lv

 */


// your network name also called SSID
char ssid[] = "Duo";
// your network password
char password[] = "password";

#if defined(ARDUINO) 
SYSTEM_MODE(MANUAL);//do not connect to cloud
#else
SYSTEM_MODE(AUTOMATIC);//connect to cloud
#endif

TCPServer server(23);

boolean alreadyConnected = false; // whether or not the client was connected previously

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

  // you're connected now, so print out the status:
  printWifiStatus();

  // start the server:
  server.begin();
}


void loop() {
  // wait for a new client:
  TCPClient client = server.available();


  // when the client sends the first byte, say hello:
  if (client) {
    if (!alreadyConnected) {
      // clead out the input buffer:
      client.flush();
      Serial.println("We have a new client");
      client.println("Hello, client!");
      alreadyConnected = true;
    }
    
    while (client.connected()) {
        if (client.available()) {
          // read the bytes incoming from the client:
          char thisChar = client.read();
          // echo the bytes back to the client:
          server.write(thisChar);
          // echo the bytes to the server as well:
          Serial.write(thisChar);
        }
    }
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


