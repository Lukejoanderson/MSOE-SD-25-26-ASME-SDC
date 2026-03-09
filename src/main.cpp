#include <Arduino.h>
#include <SoftwareSerial.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include <AsyncTCP.h>
#include <WiFiServer.h>
#include <ESPAsyncWebServer.h>
#include <vector>

#define WifiName "MSOE-ASME-BOT"
#define Pword "123456789"

// put function declarations and global variables here:

/*IPAddress local_IP(1,2,3,4);
IPAddress gateway(192,168,4,9);
IPAddress subnet(255,255,255,0);*/
AsyncWebServer server(80);

static AsyncWebSocketMessageHandler wsHand;
static AsyncWebSocket ws("/ws",wsHand.eventHandler());

class Motor
{
  private:
  int throttlePin;
  int dirPin;
  bool rev;
  public:
  Motor(){}//this should be fine...
  Motor(int pinThrottle, int pinDir, bool reverse)
  {
    throttlePin=pinThrottle;
    dirPin=pinDir;
    rev=reverse;
    pinMode(throttlePin,OUTPUT);
    pinMode(dirPin,OUTPUT);
  }
  void write(float speed,bool forward)
  {
    if (!rev)
    {
      digitalWrite(dirPin,forward);
    }
    else
    {
      digitalWrite(dirPin,!forward);
    }
    analogWrite(throttlePin,255*speed);
  }
  
};

class Steering
{
  private:
  Motor motorL;
  Motor motorR;
  public:
  Steering(Motor motL, Motor motR)
  {
    motorL=motL;
    motorR=motR;
  }
  void control(float x,float y)
  {
    double ang = atan2(abs(y),abs(x));
    float speed=sqrt(x*x+y*y);
    if (x>=0&&y>=0)
    {
      motorL.write(speed,true);
      motorR.write(speed*(ang/PI),true);
    }
    else if (y>=0&&x<=0)
    {
      motorL.write(speed*ang/PI,true);
      motorR.write(speed,true);
    }
    else if (y<=0&&x<=0)
    {
      motorL.write(speed*ang/PI,false);
      motorR.write(speed,false);
    }
    else if (y<=0&&x>=0)
    {
      motorL.write(speed,false);
      motorR.write(speed*(ang/PI),false);
    }
  }
};

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
  void dimLeg(float percent)
  {
    analogWrite(boardLed, int(255*percent));
    boardLedOn=true;
  }
  Bot()
  {
    pinMode(boardLed,OUTPUT);
  }
};

Bot trashBot;
Motor LeftMotor(14,32,true);
Motor RightMotor(15,33,false);
Steering Drivebase(LeftMotor,RightMotor);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200); //for debug
  Serial.println("Hello World");
  
  //WiFi.softAPConfig(local_IP,gateway,subnet); this breaks async for some reason.
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
    ws.textAll("ready");
  });

    wsHand.onMessage([](AsyncWebSocket *server, AsyncWebSocketClient *client, const uint8_t *data, size_t len) {
      String msg = String((char *)data);
      //Serial.println(msg);
      int length=msg.length();
      int prevSub=-1;
      int dataSeg=0; //which section of sent data is being handled. 0-forwards/backwards 1-left/right 2-arm angle 3-arm dist 4-arm height (Future probably: 5-11, buttons 1-6)
      for (int i=0; i<length; i++)
      {
          if (msg.charAt(i)==',')
          {
            String sub=msg.substring(prevSub+1,i);
            prevSub=i;
            float forward;
            switch (dataSeg)
            {
              case 0:
                forward=sub.toFloat();
                break;
              case 1:
                Drivebase.control(sub.toFloat(),forward);
                break;
              case 4:
                trashBot.dimLeg(sub.toFloat());
              break;
              default:
              break;
            }
            dataSeg++;
            //Serial.println(sub);
          }
      }
      ws.textAll("ready");
  });

  server.addHandler(&ws);
  server.begin();
}





void loop() {
  ws.cleanupClients();
  delay(1000);
  //probably should do some more smart stuff here (proper timer stuff), this is probably where the sorting code is going to go. Also maybe if we need to do motion smoothing for the arm servos.
}

// put function definitions here:

