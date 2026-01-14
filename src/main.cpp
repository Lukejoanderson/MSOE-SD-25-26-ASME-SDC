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

/*IPAddress local_IP(1,2,3,4);
IPAddress gateway(192,168,4,9);
IPAddress subnet(255,255,255,0);*/
AsyncWebServer server(80);

static AsyncWebSocketMessageHandler wsHand;
static AsyncWebSocket ws("/ws",wsHand.eventHandler());



class Bot
{
  private:
  const int boardLed=13;
  bool boardLedOn=false;
  public:
  void toggleLed()
  {
    if (boardLedOn)
    {
      digitalWrite(boardLed,LOW);
      boardLedOn=false;
    }
    else
    {
      digitalWrite(boardLed,HIGH);
      boardLedOn=true;
    }
  }
  Bot()
  {
    pinMode(boardLed,OUTPUT);
  }
};

Bot trashBot;

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
    request->send(SPIFFS,"/page.html","text/html");
  });

  server.on("/css.css", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send(SPIFFS,"/css.css");
  });

  server.on("/js.js", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send(SPIFFS,"/js.js");
  });

  server.onNotFound([](AsyncWebServerRequest *request){
    request->send(404);
  });

  //websocket server stuff
  wsHand.onConnect([](AsyncWebSocket *server, AsyncWebSocketClient *client) {
    Serial.println("WS connected");
  });

    wsHand.onMessage([](AsyncWebSocket *server, AsyncWebSocketClient *client, const uint8_t *data, size_t len) {
      trashBot.toggleLed();
  });

  server.addHandler(&ws);
  server.begin();
}





void loop() {
  ws.cleanupClients();
}

// put function definitions here:

