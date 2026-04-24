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
Servo gate;
Servo dump;
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
      motorL.write(1,true);
      motorR.write(1,true);
    }
    else if (y<=-0.95)
    {
      motorL.write(1,false);
      motorR.write(1,false);
    }
    else if (x>=0.95)
    {
      motorL.write(1,true);
      motorR.write(1,false);
    }
    else if (x<=-0.95)
    {
      motorL.write(1,false);
      motorR.write(1,true);
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
            sortserv.write(135);
          }
          else
          {
            sortserv.write(50);
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
    Servo twistServo;
    Servo shoulderServo;
    Servo elbowServo;
    Servo wristServo;
    Servo gripperServo;

    const int twistServoPin    = A0;
    const int shoulderServoPin = A1;
    const int elbowServoPin = A5;
    const int wristServoPin = SCK;
    const int gripperServoPin = MOSI;

    // Servo limits (LOGICAL angles)
    const int twistServoMin =  0;
    const int twistServoMax =  180;
    const int twistServoHome = 90;
    int currTwistServoAngle = twistServoHome;

    const int shoulderServoMin = 60;
    const int shoulderServoMax = 180;
    const int shoulderServoHome = 75;
    const int shoulderServoOffset = 0;
    int currShoulderServoAngle = shoulderServoHome;

    const int elbowServoMin = 0;
    const int elbowServoMax = 180;
    const int elbowServoHome = 60;
    const int elbowServoOffset = 3;
    int currElbowServoAngle = elbowServoHome;

    const int wristServoMin = 0;
    const int wristServoMax = 180;
    const int wristServoHome = 0;
    const int wristServoOffset = 12;
    int currWristServoAngle = wristServoHome;

    const int gripperServoMin = 45;
    const int gripperServoMax = 120;
    const int gripperServoHome = 45;
    int currGripperServoAngle = gripperServoHome;

    // Gripper Stuff
    NAU7802 loadcell;
    const int LoadCellCutoff = 1000;

    enum GripperState {
      GRIPPER_IDLE,
      GRIPPER_CLOSING,
      GRIPPER_HOLDING,
      GRIPPER_OPENING
    };

    GripperState gripperState = GRIPPER_IDLE;

    unsigned long stateStartTime = 0;
    unsigned long lastMove = 0;
    unsigned long lastForceRead = 0;
    int force = 0;

    // -------- Dump FSM --------
    enum DumpState {
      DUMP_IDLE,
      DUMP_TWIST,
      DUMP_ARM,
      DUMP_WAIT_BEFORE_DUMP,
      DUMP_DUMP,
      DUMP_WAIT_AFTER_DUMP,
      DUMP_RETURN_ARM,
      DUMP_RETURN_TWIST
    };

    DumpState dumpState = DUMP_IDLE;

    const int dumpShoulderAngle = 120;  // <-- set yourself
    const int dumpElbowAngle    = 15;  // <-- set yourself
    const int dumpTwistAngle    = twistServoHome;

    int storedTwist;
    int storedShoulder;
    int storedElbow;

    unsigned long dumpTimer = 0;
    unsigned long dumpLastMove = 0;
    const int dumpMoveDelay = 50;

    int lastTwist = -999;
    int lastShoulder = -999;
    int lastElbow = -999;
    int lastWrist = -999;
    int lastGripper = -999;

    void writeTwist(int angle){
      int corrected = constrain(angle, twistServoMin, twistServoMax);
      if (abs(corrected - lastTwist) > 0){
        twistServo.write(corrected);
        lastTwist = corrected;
      }
      currTwistServoAngle = angle;
    }

    void writeShoulder(int angle){
      int corrected = constrain(angle + shoulderServoOffset, shoulderServoMin, shoulderServoMax);
      if (abs(corrected - lastShoulder) > 0){
        shoulderServo.write(corrected);
        lastShoulder = corrected;
      }
      currShoulderServoAngle = angle;
    }

    void writeElbow(int angle){
      int corrected = constrain(angle + elbowServoOffset, elbowServoMin, elbowServoMax);
      if (abs(corrected - lastElbow) > 0){
        elbowServo.write(corrected);
        lastElbow = corrected;
      }
      currElbowServoAngle = angle;
    }

    void writeWrist(int angle){
      int corrected = constrain(angle + wristServoOffset, wristServoMin, wristServoMax);
      if (abs(corrected - lastWrist) > 0){
        wristServo.write(corrected);
        lastWrist = corrected;
      }
      currWristServoAngle = angle;
    }

  public:
  Arm(){}

  void setup(){
    Serial.println("Waiting for Load Cell! ");
    unsigned long wait = millis();
    while (!loadcell.begin() && (millis() - wait) < 5000) {
      Serial.print(".");
      delay(100);
    }

    Serial.println();
    if (millis() - wait >= 5000) {
        Serial.println("Load Cell FAILED to connect.");
        currGripperServoAngle = gripperServoHome;
    } else {
        Serial.println("Load Cell Connected!");
    }

    loadcell.setSampleRate(10);
    loadcell.setGain(1);
    loadcell.calibrateAFE();
    delay(1000);
    loadcell.calculateZeroOffset(50);
    delay(500);

    //allAttach();
    //allHome();
  }

  void allAttach(){
    twistServo.attach(twistServoPin);
    shoulderServo.attach(shoulderServoPin);
    elbowServo.attach(elbowServoPin);
    wristServo.attach(wristServoPin);
    gripperServo.attach(gripperServoPin);
  }

  void allHome(){
    writeTwist(twistServoHome);
    writeShoulder(shoulderServoHome);
    writeElbow(elbowServoHome);
    writeWrist(wristServoHome);
    gripperServo.write(gripperServoHome);
  }

  void startDump(){
    Serial.println("START DUMP CALLED");

    if (gripperState != GRIPPER_HOLDING) return;
    if (dumpState != DUMP_IDLE) return;

    Serial.println("DUMP STARTED");

    storedTwist = currTwistServoAngle;
    storedShoulder = currShoulderServoAngle;
    storedElbow = currElbowServoAngle;

    dumpState = DUMP_TWIST;
  }

  int readForce(){
    int32_t reading = loadcell.getReading() - loadcell.getZeroOffset();

    static unsigned long lastPrint = 0;
    if (millis() - lastPrint >= 250) {
      Serial.print("Load Cell Reading: ");
      Serial.println(reading);
      lastPrint = millis();
    }

    return reading;
  }

  void updateArm(){

    const int WRIST_LEVEL_CONST = -(shoulderServoHome + elbowServoHome);

    int desiredWristAngle = WRIST_LEVEL_CONST + (currShoulderServoAngle+shoulderServoOffset) + (currElbowServoAngle+elbowServoOffset);
    desiredWristAngle = constrain(desiredWristAngle, wristServoMin, wristServoMax);

    if (dumpState == DUMP_IDLE){
      writeWrist(desiredWristAngle);
    }

    // -------- Dump FSM --------

    switch (dumpState)
    {
      case DUMP_IDLE:
        break;

      case DUMP_TWIST:
        if (millis() - dumpLastMove > dumpMoveDelay){
          if (currTwistServoAngle != dumpTwistAngle){
            int dir = (dumpTwistAngle > currTwistServoAngle) ? 1 : -1;
            writeTwist(currTwistServoAngle + dir);
          } else {
            dumpState = DUMP_ARM;
          }
          dumpLastMove = millis();
        }
        break;

      case DUMP_ARM:
        if (millis() - dumpLastMove > dumpMoveDelay){
          bool done = true;

          if (currShoulderServoAngle != dumpShoulderAngle){
            int dir = (dumpShoulderAngle > currShoulderServoAngle) ? 1 : -1;
            writeShoulder(currShoulderServoAngle + dir);
            done = false;
          }

          if (currElbowServoAngle != dumpElbowAngle){
            int dir = (dumpElbowAngle > currElbowServoAngle) ? 1 : -1;
            writeElbow(currElbowServoAngle + dir);
            done = false;
          }

          if (done){
            dumpState = DUMP_WAIT_BEFORE_DUMP;
            dumpTimer = millis();
          }

          dumpLastMove = millis();
        }
        break;

      case DUMP_WAIT_BEFORE_DUMP:
        if (millis() - dumpTimer > 1000){
          dumpState = DUMP_DUMP;
        }
        break;

      case DUMP_DUMP:
        // Move wrist to dump (OVERRIDE leveling)
        writeWrist(120);  // <-- TUNE THIS FOR DUMP
        dumpTimer = millis();
        dumpState = DUMP_WAIT_AFTER_DUMP;
        break;

      case DUMP_WAIT_AFTER_DUMP:
        if (millis() - dumpTimer > 1000){
          writeWrist(desiredWristAngle);
          dumpState = DUMP_RETURN_ARM;
        }
        break;

      case DUMP_RETURN_ARM:
        if (millis() - dumpLastMove > dumpMoveDelay){
          bool done = true;

          if (currShoulderServoAngle != storedShoulder){
            int dir = (storedShoulder > currShoulderServoAngle) ? 1 : -1;
            writeShoulder(currShoulderServoAngle + dir);
            done = false;
          }

          if (currElbowServoAngle != storedElbow){
            int dir = (storedElbow > currElbowServoAngle) ? 1 : -1;
            writeElbow(currElbowServoAngle + dir);
            done = false;
          }

          if (done){
            dumpState = DUMP_RETURN_TWIST;
          }

          dumpLastMove = millis();
        }
        break;

      case DUMP_RETURN_TWIST:
        if (millis() - dumpLastMove > dumpMoveDelay){
          if (currTwistServoAngle != storedTwist){
            int dir = (storedTwist > currTwistServoAngle) ? 1 : -1;
            writeTwist(currTwistServoAngle + dir);
          } else {
            dumpState = DUMP_IDLE;
          }
          dumpLastMove = millis();
        }
        break;
    }

    // -------- Gripper FSM --------

    if (millis() - lastForceRead > 50){
      force = readForce();
      lastForceRead = millis();
    }

    switch (gripperState)
    {
      case GRIPPER_IDLE:
        // Do nothing, gripper is open
        break;

      case GRIPPER_CLOSING:
        if (stateStartTime == 0) stateStartTime = millis();

        // Stop if force reached
        if (force > LoadCellCutoff){
          gripperState = GRIPPER_HOLDING;
          stateStartTime = 0;
          break;
        }

        // Timeout safety
        if (millis() - stateStartTime > 5000){
          gripperState = GRIPPER_OPENING;
          stateStartTime = 0;
          break;
        }

        // Move inward
        if (millis() - lastMove > 40){
          currGripperServoAngle += 1;
          currGripperServoAngle = constrain(currGripperServoAngle, gripperServoMin, gripperServoMax);
          gripperServo.write(currGripperServoAngle);
          lastMove = millis();
        }
        break;

      case GRIPPER_HOLDING:
        // Just hold position, do nothing
        break;

      case GRIPPER_OPENING:
        if (millis() - lastMove > 20){
          currGripperServoAngle -= 2;
          currGripperServoAngle = constrain(currGripperServoAngle, gripperServoMin, gripperServoMax);
          gripperServo.write(currGripperServoAngle);
          lastMove = millis();
        }

        // Done opening
        if (currGripperServoAngle <= gripperServoMin){
          gripperState = GRIPPER_IDLE;
        }
        break;
    }
  }


  void openGripper(){
    gripperState = GRIPPER_OPENING;
    stateStartTime = 0;
  }

  void toggleGripper(){
    if (gripperState == GRIPPER_IDLE){
      gripperState = GRIPPER_CLOSING;
      stateStartTime = 0;
    }
    else {
      gripperState = GRIPPER_OPENING;
      stateStartTime = 0;
    }
  }

  void incrementTwist(bool pressed, int dir){
    static unsigned long holdStart = 0;
    static unsigned long last = 0;

    if (!pressed) { holdStart = 0; return; }
    if (holdStart == 0) { holdStart = millis(); }

    unsigned long holdTime = millis() - holdStart;
    int step = (1 + holdTime / 2) * dir;

    if (millis() - last > 10) {
      writeTwist(currTwistServoAngle + step);
      last = millis();
    }
  }

  void incrementShoulder(bool pressed, int dir){
    static unsigned long holdStart = 0;
    static unsigned long last = 0;

    if (!pressed) { holdStart = 0; return; }
    if (holdStart == 0) { holdStart = millis(); }

    unsigned long holdTime = millis() - holdStart;
    int step = (1 + holdTime / 2) * dir;

    if (millis() - last > 10) {
      int next = currShoulderServoAngle + step;
      next = constrain(next, shoulderServoMin, shoulderServoMax);
      writeShoulder(next);
      last = millis();
    }
  }

  void incrementElbow(bool pressed, int dir){
    static unsigned long holdStart = 0;
    static unsigned long last = 0;

    if (!pressed) { holdStart = 0; return; }
    if (holdStart == 0) { holdStart = millis(); }

    unsigned long holdTime = millis() - holdStart;
    int step = (1 + holdTime / 2) * dir;

    if (millis() - last > 10) {
      int next = currElbowServoAngle + step;
      next = constrain(next, elbowServoMin, elbowServoMax);
      writeElbow(next);
      last = millis();
    }
  }
};





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
  gate.attach(13);
  dump.attach(12);
  gate.write(90);
  dump.write(0);
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
            bool trash;
            switch (dataSeg)
            {
              case 0:
                forward=sub.toFloat();
                break;
              case 1:
                Drivebase.control(sub.toFloat(),forward);
                break;
              case 4:
                //trashBot.dimLeg(sub.toFloat());
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
              case 14:
                trashBotArm.incrementTwist(sub.toInt(),1);
                break;
              case 11:
                trashBotArm.incrementTwist(sub.toInt(),-1);
                break;
              case 15:
                trashBotArm.incrementShoulder(sub.toInt(),1);
                break;
              case 12:
                trashBotArm.incrementShoulder(sub.toInt(),-1);
                break;
              case 16:
                trashBotArm.incrementElbow(sub.toInt(),1);
                break;
              case 13:
                trashBotArm.incrementElbow(sub.toInt(),-1);
                break;
              case 17:
              //Serial.println("GRIPPER TOGGLE RECEIVED");
              {
                static bool lastState = false;
                bool current = sub.toInt();

                if (current && !lastState) {
                  trashBotArm.toggleGripper();
                }

                lastState = current;
              }
              break;

              case 18:
                Serial.println("DUMP BUTTON RECEIVED");
                trashBotArm.startDump();
                break;
              case 8:
                if(sub.toInt())
                {
                  dump.write(55);
                  sort.active=false;
                }
                else
                {
                  dump.write(0);
                }
              break;
              case 9:
                trash=sub.toInt();
                break;
              case 10:
              if(trash){
                gate.write(0);
              }
              else if(sub.toInt())
              {
                gate.write(180);
              }
              else
              {
                gate.write(90);
              }
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
  trashBotArm.updateArm();
  //trashBotArm.readForce();
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