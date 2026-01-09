#include <Arduino.h>
#include <SoftwareSerial.h>
#include <WiFi.h>

#define WifiName "MSOE-ASME-BOT"
#define Pword "123456789"

// put function declarations and global variables here:

WiFiServer server(80);
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200); //for debug
  Serial.println("Hello World");
  WiFi.softAP(WifiName,Pword);
  server.begin();
  Serial.println(WiFi.localIP());
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
  }
}

// put function (and class) definitions here:

//test