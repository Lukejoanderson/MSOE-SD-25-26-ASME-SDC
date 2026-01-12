#include <Arduino.h>
#include <SoftwareSerial.h>
#include <WiFi.h>
#include <string.h>
#include <SPIFFS.h>
#include <AsyncTCP.h>
#include <WiFiServer.h>
#include <ESPAsyncWebServer.h>

#define WifiName "MSOE-ASME-BOT"
#define Pword "123456789"

// put function declarations and global variables here:

IPAddress local_IP(1,2,3,4);
IPAddress gateway(192,168,4,9);
IPAddress subnet(255,255,255,0);
AsyncWebServer server(80);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200); //for debug
  Serial.println("Hello World");
  
  //WiFi.softAPConfig(local_IP,gateway,subnet);
  WiFi.softAP(WifiName,Pword);

  Serial.println(WiFi.softAPIP());
  
  SPIFFS.begin();

  //Server stuff
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
  {
  request->send(SPIFFS,"/page.html", "text/html");
  });

  server.onNotFound([](AsyncWebServerRequest *request){
    request->send(404);
  });

  server.begin();
}





void loop() {
  delay(100);
}

// put function (and class) definitions here: