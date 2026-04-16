#include <Arduino.h>
#include <SoftwareSerial.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include <AsyncTCP.h>
#include <WiFiServer.h>
#include <ESPAsyncWebServer.h>
#include <vector>
#include <Adafruit_APDS9960.h>
#include <ESP32Servo.h>
#include "SparkFun_Qwiic_Scale_NAU7802_Arduino_Library.h"
#define WifiName "MSOE-ASME-BOT"
#define Pword "123456789"

Adafruit_APDS9960 apds;
void reorgRGB(int rgb[]);
void RGB2HSV(int rgb[], double hsv[]);

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
    if (y>=0.95)
    {
      motorL.write(1,false);
      motorR.write(1,false);
    }
    else if (y<=-0.95)
    {
      motorL.write(1,true);
      motorR.write(1,true);
    }
    else if (x>=0.95)
    {
      motorL.write(1,false);
      motorR.write(0,false);
    }
    else if (x<=-0.95)
    {
      motorL.write(0,false);
      motorR.write(1,false);
    }
    else
    {
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
  }
};
class timer
{
  private:
    unsigned long int starttime;
  public:
  void start()
  {
    starttime=millis();
  }
  unsigned long int gettime()
  {
    return millis()-starttime;
  }
};
class sorter
{
  public:
  bool active=false;
  bool overrideT=false;
  bool overrideR=false;
  timer delay;
  Servo sortserv;
  unsigned short int state=0;
  sorter()
  {
    sortserv.attach(27);
    sortserv.write(90);
  }
  void update()
  {
    if(active)
    {
      switch (state)
      {
      case 0:
        sortserv.write(90);
        if(apds.readProximity()>25&&delay.gettime()>250)
        {
          state=1;
          delay.start();
        }
        break;
      case 1:
        if(delay.gettime()>=250&&apds.readProximity()>25&&apds.colorDataReady())
        {
          uint16_t r, g, b, c;
          apds.getColorData(&r, &g, &b, &c);
          int RGB[3]={r,g,b};
          reorgRGB(RGB);
          double HSV[3];
          RGB2HSV(RGB,HSV);
          if (HSV[0]>=70&&HSV[0]<=260&&HSV[1]>.4&&HSV[2]>.3)
          {
            sortserv.write(130);
          }
          else
          {
            sortserv.write(55);
          }
          delay.start();
          state=2;
        }
        break;
      case 2:
      if(delay.gettime()>500)
      {
        sortserv.write(90);
        state=0;
        delay.start();
      }
      break;
      default:
        break;
      }
    }
    else
    {
      state=0;
      delay.start();
      if(overrideT)
      {
        sortserv.write(55);
      }
      else if (overrideR)
      {
        sortserv.write(130);
      }
      else
      {
        sortserv.write(90);
      }
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
    const int twistServoPin    = A0;
    const int shoulderServoPin = A1;
    const int elbowServoPin = A5;
    const int wristServoPin = SCK;
    const int gripperServoPin = MOSI;

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
    twistServo.attach(twistServoPin);
    shoulderServo.attach(shoulderServoPin);
    elbowServo.attach(elbowServoPin);
    wristServo.attach(wristServoPin);
    gripperServo.attach(gripperServoPin);
      // Set other servos here

    delay(1000);

    twistServo.write(90);
    shoulderServo.write(90);
    elbowServo.write(90);
    wristServo.write(90);
    gripperServo.write(gripperOpenAngle);

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

<<<<<<< Updated upstream
=======


>>>>>>> Stashed changes
Bot trashBot;
Arm trashBotArm;
Motor LeftMotor(14,32,true);
Motor RightMotor(15,33,true);
Steering Drivebase(LeftMotor,RightMotor);
sorter sort;
timer looptime;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200); //for debug
  Serial.println("Hello World");

  Wire.begin();
  trashBotArm.setup();

  apds.begin();
  apds.enableColor();
  apds.enableProximity();
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
      int dataSeg=0; //which section of sent data is being handled. 0-forwards/backwards 1-left/right 2-arm angle(unused) 3-arm dist(unused) 4-arm height(unused) 5-23, buttons 1-12(buttons 1-6 are toggle, 7-12 are held)   --- 7-14 for arm
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
              case 5:
                sort.active=sub.toInt();
                break;
              case 6:
                sort.overrideR=sub.toInt();
                break;
              case 7:
                sort.overrideT=sub.toInt();
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
  looptime.start();
} 




void loop() {
  if (looptime.gettime()>=1000)
  {
    ws.cleanupClients();
    looptime.start();
  }
  sort.update();
  //probably should do some more smart stuff here (proper timer stuff), this is probably where the sorting code is going to go. Also maybe if we need to do motion smoothing for the arm servos.
}

// put function definitions here:

void reorgRGB(int rgb[])
 {
    rgb[0]=rgb[0]*5.0604+9.7535;
    rgb[0]=min(max(rgb[0],0),255);
    rgb[1]=rgb[1]*2.8581+36.832;
    rgb[1]=min(max(rgb[1],0),255);
    rgb[2]=rgb[2]*4.733-23.635;
    rgb[2]=min(max(rgb[2],0),255);
}

void RGB2HSV(int rgb[], double hsv[])
{
  //https://math.stackexchange.com/questions/556341/rgb-to-hsv-color-conversion-algorithm
  double R=rgb[0];
  double G=rgb[1];
  double B=rgb[2];
  R=R/255;
  G=G/255;
  B=B/255;
  double cmax=max(max(R,G),B);
  double cmin=min(min(R,G),B);
  double delta=cmax-cmin;
  if (R==G&&R==B)
  {
    hsv[0]=0;
  }
  else if (cmax==R)
  {
    hsv[0]=60*fmod((G-B)/delta,6);//main idea here is to turn negatives into positives that are at 300+ plus degrees, not really readable but whatever
  }
  else if (cmax==G)
  {
    hsv[0]=60*((B-R)/delta+2);
  }
  else
  {
    hsv[0]=60*((R-G)/delta+4);
  }
  if (cmax==0)
  {
    hsv[1]=0;
  }
  else
  {
    hsv[1]=delta/cmax;
  }
  hsv[2]=cmax;
  
}