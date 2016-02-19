/*
 This example prints the Wifi MAC address, and
 scans for available Wifi networks.
 Every ten seconds, it scans again. It doesn't actually
 connect to any network, so no encryption scheme is specified.
 
 Circuit:
 * Origin for CC3200 WiFi LaunchPad or CC3100 WiFi BoosterPack
   with TM4C or MSP430 LaunchPad
 * Modified for DUO Board
 
 created 1 DEC 2015
 by Jackson Lv
 */


#if defined(ARDUINO) 
SYSTEM_MODE(MANUAL);//do not connect to cloud
#else
SYSTEM_MODE(AUTOMATIC);//connect to cloud
#endif

void printEncryptionType(int thisType);
void printMacAddress();
void listNetworks();


void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(115200);

  WiFi.on();
  
  // Print WiFi MAC address:
  printMacAddress();

  // scan for existing networks:
  Serial.println("Scanning available networks...");
  listNetworks();
}

void loop() {
  delay(10000);
  // scan for existing networks:
  Serial.println("Scanning available networks...");
  listNetworks();
}

void printMacAddress() {
  // the MAC address of your Wifi
  byte mac[6];

  // print your MAC address:
  WiFi.macAddress(mac);
  Serial.print("MAC: ");
  Serial.print(mac[5], HEX);
  Serial.print(":");
  Serial.print(mac[4], HEX);
  Serial.print(":");
  Serial.print(mac[3], HEX);
  Serial.print(":");
  Serial.print(mac[2], HEX);
  Serial.print(":");
  Serial.print(mac[1], HEX);
  Serial.print(":");
  Serial.println(mac[0], HEX);
}

void listNetworks() {
  // scan for nearby networks:
  Serial.println("** Scan Networks **");
  WiFiAccessPoint aps[20];
  int found = WiFi.scan(aps, 20);
  if (found <= 0)
  {
    Serial.println("Couldn't get a wifi connection");
    while (true);
  }

  // print the list of networks seen:
  Serial.print("number of available networks:");
  Serial.println(found);

  for (int i=0; i<found; i++) {
    WiFiAccessPoint& ap = aps[i];
    Serial.print("SSID: ");
    Serial.println(ap.ssid);
    //Serial.print("Security: ");
    //printEncryptionType(ap.security);
    Serial.print("Channel: ");
    Serial.println(ap.channel);
    Serial.print("RSSI: ");
    Serial.println(ap.rssi);
  }
}

void printEncryptionType(int thisType) {
  // read the encryption type and print out the name:
  switch (thisType) {
  case UNSEC:
    Serial.println("UNSEC");
    break;
  case WEP:
    Serial.println("WEP");
    break;
  case WPA:
    Serial.println("WPA");
    break;
  case WPA2:
    Serial.println("WPA2");
    break;
  }
}




