
#include "Arduino.h"

//#define AP "Your_AP"
//#define PIN "Your_PIN_I_Do_Not_Know"

SYSTEM_MODE(MANUAL);

TCPServer server = TCPServer(80);
TCPClient client;

int led1 = D7;

boolean endsWith(char* inString, char* compString);

void setup()
{
    char addr[16];

    pinMode(led1, OUTPUT);

    Serial.begin(115200);
    Serial.println("Started");

    WiFi.on();
    WiFi.setCredentials(AP, PIN, WPA2);
    WiFi.connect();
  
    IPAddress localIP = WiFi.localIP();

    while (localIP[0] == 0)
    {
        localIP = WiFi.localIP();
        Serial.println("Waiting for an IP address...");
        delay(1000);
    }
  
    sprintf(addr, "IP Address: %u.%u.%u.%u", localIP[0], localIP[1], localIP[2], localIP[3]);
    
    Serial.println(addr);

    server.begin();
}
    
void loop()
{
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

