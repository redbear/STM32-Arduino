#include "Arduino.h"
#include "MDNS.h"

//SYSTEM_MODE(MANUAL);

TCPServer server = TCPServer(80);
TCPClient client;
MDNS mdns;

int led1 = D7;

boolean endsWith(char* inString, char* compString);
void mdns_init();

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

void setup()
{
    pinMode(led1, OUTPUT);

    Serial.begin(115200);
    delay(5000);

    Serial.println("Arduino sketch started.\n");
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

