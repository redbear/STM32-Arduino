/* 
 *  This is a simple TCP server, telnet to this sketch with an IP 
 *  assigned.
 *  e.g. telnet 192.168.0.5 8888
 * 
 */
 
// Modified the following for your AP/Router.
//#define AP "duo"
//#define PIN "password"

#if defined(ARDUINO) 
SYSTEM_MODE(MANUAL);//do not connect to cloud
#else
SYSTEM_MODE(AUTOMATIC);//connect to cloud
#endif

// Server Port
TCPServer server = TCPServer(8888);
TCPClient client;
    
void setup()
{
    char addr[16];

    Serial.begin(115200);
    delay(5000);
    
    WiFi.on();
    WiFi.setCredentials(AP, PIN, WPA2);
    WiFi.connect();
  
    IPAddress localIP = WiFi.localIP();

    while (localIP[0] == 0)
    {
        localIP = WiFi.localIP();
        Serial.println("waiting for an IP address");
        delay(1000);
    }
  
    sprintf(addr, "%u.%u.%u.%u", localIP[0], localIP[1], localIP[2], localIP[3]);
    
    Serial.println(addr);

    server.begin();
}
    
void loop()
{
    if (client.connected())
    {
        Serial.println("Connected by TCP client.");
        client.println("Hello!");
    }
    else
    {
        client = server.available();
    }

    delay(2000);
}

