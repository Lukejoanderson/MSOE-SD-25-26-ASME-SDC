#include <Arduino.h>
#include <SoftwareSerial.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include <AsyncTCP.h>
#include <WiFiServer.h>
#include <ESPAsyncWebServer.h>
#include <vector>
#include "SparkFun_Qwiic_Scale_NAU7802_Arduino_Library.h"
#include <ESP32Servo.h>

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

class Arm
{
  private:
    // Placeholder pin numbers, need to be changed to actual pins
    const int twistServoPin    = 0;
    const int shoulderServoPin = 0;
    const int elbowServoPin = 0;
    const int wristServoPin = 0;
    const int gripperServoPin = 0;

    Servo twistServo;
    Servo shoulderServo;
    Servo elbowServo;
    Servo wristServo;
    Servo gripperServo;

    // Servo limits in degrees. Still need to fix some and ensure bot is on these
    const int twistServoMin = -90;
    const int twistServoMax =  90;
    const int shoulderServoMin = -90;
    const int shoulderServoMax = 90;
    const int elbowServoMin = -60;
    const int elbowServoMax = 120;
    const int wristServoMin = -90;
    const int wristServoMax =  90;

    // Gripper Stuff
    bool gripperClosed = false;
    const int LoadCellCutoff = 5000; // Test value
    int gripperOpenAngle = 90; // Test value, may need to be changed
    int currGripperAngle = 90;
    NAU7802 loadcell;

  public:
  Arm(){}
  void setup(){
    // Load Cell setup
    Serial.println("Waiting for Load Cell!");
    while (!loadcell.begin()) {Serial.print("."); delay(100);}
    Serial.println(); Serial.println("Load Cell Connected!");

    loadcell.setSampleRate(10);
    loadcell.setGain(128);
    loadcell.calibrateAFE();
    delay(500);
    loadcell.calculateZeroOffset(50);

    // Servo setup
      // gripper
    twistServo.setPeriodHertz(50);
    twistServo.attach(twistServoPin, 500, 2400);

    shoulderServo.setPeriodHertz(50);
    shoulderServo.attach(shoulderServoPin, 500, 2400);

    elbowServo.setPeriodHertz(50);
    elbowServo.attach(elbowServoPin, 500, 2400);

    wristServo.setPeriodHertz(50);
    wristServo.attach(wristServoPin, 500, 2400);

    gripperServo.setPeriodHertz(50);
    gripperServo.attach(gripperServoPin, 500, 2400);
      // Set other servos here

    // whatever
  }

  int readForce(){
    int32_t reading = loadcell.getReading() - loadcell.getZeroOffset();
    //Serial.print("Reading: ");
    //Serial.println(reading);
    return reading;
  }

  void closeGripper(){
    while (readForce() < LoadCellCutoff){
      currGripperAngle += 1; // Test value, may need to be changed
      gripperServo.write(currGripperAngle);
      delay(100); // Test value, may need to be changed
    }
    gripperClosed = true;
  }

  void openGripper(){
    currGripperAngle = gripperOpenAngle;
    gripperServo.write(currGripperAngle);
    gripperClosed = false;
  }

  

  void moveAround(){

  }

  // Initialize servos and strain gauge here
  // twistServo.attach(twistServoPin);
  // shoulderServo.attach(shoulderServoPin);
  // elbowServo.attach(elbowServoPin);
  // wristServo.attach(wristServoPin);
  // gripperServo.attach(gripperServoPin);

  // Need to set up strain gauge pin as input and calibrate it

  // Commands needed:
  // 1. Set twist angle
  // 2. Move out or in
  // 3. Move up or down
  // 4. Open or close gripper
  // 5. Bring arm to dump position
  // 6. Dump
  // 7. Bring arm to some average pickup position (or the last pickup spot?)
  // 8. ??

};

//Initialize bot stuff
Bot trashBot;
Motor LeftMotor(14,32,true);
Motor RightMotor(15,33,false);
Steering Drivebase(LeftMotor,RightMotor);
Arm trashArm;

//Total bot set-up command
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200); //for debug
  Serial.println("Hello World");
  Wire.begin();
  trashArm.setup();
  
  //WiFi.softAPConfig(local_IP,gateway,subnet); // this breaks async for some reason.
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
  trashArm.readForce();
}

// put function definitions here:

