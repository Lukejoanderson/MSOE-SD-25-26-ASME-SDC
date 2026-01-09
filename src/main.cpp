#include <Arduino.h>
#include <SoftwareSerial.h>
#include <WiFi.h>

#define WifiName "MSOE-ASME-BOT"
#define Pword "123456789"

// put function declarations and global variables here:

IPAddress local_IP(1,2,3,4);
IPAddress gateway(192,168,4,9);
IPAddress subnet(255,255,255,0);

WiFiServer server(80);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200); //for debug
  Serial.println("Hello World");
  WiFi.softAPConfig(local_IP,gateway,subnet);
  WiFi.softAP(WifiName,Pword);
  server.begin();
  Serial.println(WiFi.softAPIP());
}

void loop() {
  WiFiClient client=server.available();
  if (client.available())
  {
    int bytes=client.available();
    char message[bytes];
    for (int i=0;i<bytes;i++)
    {
      message[i]=client.read();
    }
    Serial.write(message);
    client.println("HTTP/1.1 200 OK");
    client.println("Content-type:text/html");
    client.println("Connection: close");
    client.println();
    client.println("<!DOCTYPE html><html><body><h1>Hi</h1></body></html>");
    client.println();
  }
}

// put function (and class) definitions here: