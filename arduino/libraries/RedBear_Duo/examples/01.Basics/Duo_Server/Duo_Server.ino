/* 
 *  This is a simple TCP server, telnet to this sketch with an IP 
 *  assigned.
 *  e.g. telnet 192.168.0.5 8888
 * 
 */
 
// Modified the following for your AP/Router.
//#define AP "Duo"
//#define PIN "password"

#if defined(ARDUINO) 
SYSTEM_MODE(MANUAL);//do not connect to cloud
#else
SYSTEM_MODE(AUTOMATIC);//connect to cloud
#endif

#define MAX_CLIENT_NUM   3
// Server Port
TCPServer server = TCPServer(8888);
TCPClient client[MAX_CLIENT_NUM];
    
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
    for(uint8_t i = 0;i<MAX_CLIENT_NUM;i++)
    {
        if (client[i].connected())
        {
            Serial.println("Connected by TCP client.");
            client[i].println("Hello!");
        }
        else
        {
            client[i] = server.available();
        }
    }

    delay(2000);
}

