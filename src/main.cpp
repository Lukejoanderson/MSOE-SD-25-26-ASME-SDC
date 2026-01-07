#include <Arduino.h>
#include <SoftwareSerial.h>
#include <WiFi.h>

// put function declarations and global variables here:

WiFiServer server(80);
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200); //for debug
  Serial.write("Hello World");
  WiFi.softAP("MSOE-ASME-BOT","123456789");
  server.begin();
}

void loop() {
  // put your main code here, to run repeatedly:
}

// put function (and class) definitions here:

//test