#include <LiquidCrystal_I2C.h>
#include <string.h>
#include "Queue.h"
#include "Timer.h"

struct task{
    int state;
    unsigned long period;
    unsigned long elapsedTime;
    int (*TickFct)(int);
};

int noiseCnt = 0;
//LCD Address: 0x27, 16 columns, 2 rows
LiquidCrystal_I2C LCD(0x27, 16, 2);
const int MAX_OCCUPANCY = 5;
int occupancy = 0;
bool isNoise = false;
bool isMaxOccu = false;
bool isMotion = false;
Queue<int> msgQ;
Queue<int> msgQRPi;

//Task Scheduler Data Structure
task tasks[7];
const unsigned short tasksNum = 7;
const unsigned long tasksPeriodGCD = 50;

enum MicSM_States {MicSM_Start, MicSM_Detection, MicSM_SetResult};
enum MotionSM_States {MotionSM_Start, MotionSM_Detection, MotionSM_SetResult};
enum NotifSM_States {NotifSM_Start, NotifSM_Sound, NotifSM_maxOccu};
enum RPiSM_States {RPiSM_Start};
enum OccupancySM_States {OccupancySM_Start, OccupancySM_Update};
enum LEDSM_States {LEDSM_Start};
enum BBSM_States {BBSM_Start, BBSM_BB1, BBSM_BB2, BBSM_BB1_, BBSM_BB2_};

int TickFct_MicSM(int MicSM_State); //DONE! Waiting for testing
int TickFct_NotifSM(int NotifSM_State); //DONE! Waiting for testing
int TickFct_RPiSM(int RPiSM_State); //DONE! Waiting for testing
int TickFct_OccupancySM(int OccupancySM_State); //DONE! Waiting for testing
int TickFct_MotionSM(int MotionSM_State); //DONE! Waiting for testing
int TickFct_LEDSM(int LightSM_State); //DONE! Waiting for testing
int TickFct_BBSM(int BBSM_State); //DONE! Waiting for testing

void setup() {
  Serial.begin(9600); //for serial debugging
  pinMode(A0, INPUT); //for microphone
  pinMode(7, INPUT_PULLUP); //for button
  pinMode(8, OUTPUT); //for buzzer
  pinMode(9, OUTPUT); //for LED
  pinMode(10, INPUT_PULLUP); //for PIR Motion Sensor
  //NOTE: Pullup resistors NEEDED for breakbeam to work!
  pinMode(11, INPUT_PULLUP); //for breakbeam sensor 1
  pinMode(12, INPUT_PULLUP); //for breakbeam sensor 2
  digitalWrite(9, LOW); //turn off LED
   
  LCD.begin(); //LCD Setup
  LCD.setBacklight(HIGH);
  LCD.clear();
  LCD.print("Occupancy: " + String(occupancy));

  //initialize tasks
  unsigned char i = 0;
  tasks[i].state = MicSM_Start;
  tasks[i].period = 100;
  tasks[i].elapsedTime = tasks[i].period;
  tasks[i].TickFct = &TickFct_MicSM;

  i++;
  tasks[i].state = NotifSM_Start;
  tasks[i].period = 500;
  tasks[i].elapsedTime = tasks[i].period;
  tasks[i].TickFct = &TickFct_NotifSM;

  i++;
  tasks[i].state = RPiSM_Start;
  tasks[i].period = 50;
  tasks[i].elapsedTime = tasks[i].period;
  tasks[i].TickFct = &TickFct_RPiSM;

  i++;
  tasks[i].state = OccupancySM_Start;
  tasks[i].period = 100;
  tasks[i].elapsedTime = tasks[i].period;
  tasks[i].TickFct = &TickFct_OccupancySM;

  i++;
  tasks[i].state = MotionSM_Start;
  tasks[i].period = 500;
  tasks[i].elapsedTime = tasks[i].period;
  tasks[i].TickFct = &TickFct_MotionSM;

  i++;
  tasks[i].state = LEDSM_Start;
  tasks[i].period = 100;
  tasks[i].elapsedTime = tasks[i].period;
  tasks[i].TickFct = &TickFct_LEDSM;

  i++;
  tasks[i].state = BBSM_Start;
  tasks[i].period = 100;
  tasks[i].elapsedTime = tasks[i].period;
  tasks[i].TickFct = &TickFct_BBSM;

  TimerSet(tasksPeriodGCD);
  TimerOn();
}

void loop() {
  for(unsigned i = 0; i < tasksNum; i++){
    if(tasks[i].elapsedTime >= tasks[i].period){
      tasks[i].state = tasks[i].TickFct(tasks[i].state);
      tasks[i].elapsedTime = 0;
    }
    tasks[i].elapsedTime += tasksPeriodGCD;
  }
}

int TickFct_MicSM(int MicSM_State){
  // Serial.println("MicSM_State: " + String(MicSM_State));
  static unsigned int maxNoise;
  static unsigned int minNoise;
  static unsigned i;
  unsigned int sample, noise;
  switch(MicSM_State){
    case MicSM_Start:
      MicSM_State = MicSM_Detection;
      maxNoise = 0;
      minNoise = 1024;
      i = 0;
      break;
    case MicSM_Detection:
      if(i >= 1000){
        MicSM_State = MicSM_SetResult;
        break;
      }else{
        MicSM_State = MicSM_Detection;
      }
      break;
    case MicSM_SetResult:
      MicSM_State = MicSM_Detection;
      maxNoise = 0;
      minNoise = 1024;
      i = 0;
      break;
    default:
      MicSM_State = MicSM_Start;
      break;
  }
  switch(MicSM_State){
    case MicSM_Detection:
      sample = analogRead(A0);
      if(sample > maxNoise){
        maxNoise = sample;
      }else if(sample < minNoise){
        minNoise = sample;
      }
      i++;
      break;
    case MicSM_SetResult:
      noise = ((maxNoise - minNoise) * 5) / 1024;
      // Serial.println("Noise: " + String(noise));
      if(noise > 2){
        isNoise = true;
      }
      break;
    default:
      break;
  }
  return MicSM_State;
}

int TickFct_NotifSM(int NotifSM_State){
  static bool ringBuzzer = false;
  switch(NotifSM_State){
    case NotifSM_Start:
      //isNoise takes priority over isMaxOccu
      if(isNoise){
        Serial.println("Noise Detected!");
        LCD.clear();
        LCD.print("Noise detected!");
        LCD.setCursor(0, 1);
        LCD.print("Push btn to stop");
        LCD.setCursor(0, 0);
        ringBuzzer = false;
        NotifSM_State = NotifSM_Sound;
      }else if(isMaxOccu){
        LCD.clear();
        LCD.print("Max occupancy!");
        ringBuzzer = false;
        NotifSM_State = NotifSM_maxOccu;
      }
      else{
        NotifSM_State = NotifSM_Start;
      }
      break;
    case NotifSM_Sound:
      if(digitalRead(7) == HIGH){
        noTone(8);
        isNoise = false;
        LCD.clear();
        LCD.print("Occupancy: " + String(occupancy));
        NotifSM_State = NotifSM_Start;
      }
      break;
    case NotifSM_maxOccu:
      if(!isMaxOccu){
        LCD.clear();
        LCD.print("Occupancy: " + String(occupancy));
        noTone(8);
        NotifSM_State = NotifSM_Start;
      }else{
        NotifSM_State = NotifSM_maxOccu;
      }
      break;
    default:
      NotifSM_State = NotifSM_Start;
      break;
  }
  switch(NotifSM_State){
    case NotifSM_Sound:
      ringBuzzer = !ringBuzzer;
      if(ringBuzzer){
        tone(8, 1000);
      }else{
        noTone(8);
      }
      break;
    case NotifSM_maxOccu:
      tone(8, 1000);
      break;
    default:
      break;
  }
  return NotifSM_State;
}

int TickFct_RPiSM(int RPiSM_State){
  String msg = "";
  switch(RPiSM_State){
    case RPiSM_Start:
      RPiSM_State = RPiSM_Start;
      break;
    default:
      RPiSM_State = RPiSM_Start;
      break;
  }
  switch(RPiSM_State){
    case RPiSM_Start:
      if(Serial.available()){
        msg = Serial.readString();
        if(msg == "1"){
          msgQRPi.enqueue(1);
        }else if(msg == "-1"){
          msgQRPi.enqueue(-1);
        }else{
          Serial.println("Invalid message received from RPi!");
        }
      }
      break;
    default:
      break;
  }
  return RPiSM_State;
}

int TickFct_OccupancySM(int OccupancySM_State){
  int msg = 0;
  switch(OccupancySM_State){
    case OccupancySM_Start:
      if(!msgQ.empty() || !msgQRPi.empty()){
        OccupancySM_State = OccupancySM_Update;
      }else{
        OccupancySM_State = OccupancySM_Start;
      }
      break;
    case OccupancySM_Update:
      if(!msgQ.empty() || !msgQRPi.empty()){
        OccupancySM_State = OccupancySM_Update;
      }else{
        if(occupancy >= MAX_OCCUPANCY){
          isMaxOccu = true;
          LCD.clear();
          LCD.print("Max occupancy!");
        }else{
          isMaxOccu = false;
          LCD.clear();
          LCD.print("Occupancy: " + String(occupancy));
        }
        OccupancySM_State = OccupancySM_Start;
      }
      break;
    default:
      OccupancySM_State = OccupancySM_Start;
      break;
  }
  switch(OccupancySM_State){
    case OccupancySM_Update:
      if(!msgQ.empty()){
        msg = msgQ.dequeue();
        occupancy += msg;
      }
      if(!msgQRPi.empty()){
        msg = msgQRPi.dequeue();
        occupancy += msg;
      }
      break;
    default:
      break;
  }

  return OccupancySM_State;
}

int TickFct_MotionSM(int MotionSM_State){
  switch(MotionSM_State){
    case MotionSM_Start:
      MotionSM_State = MotionSM_Detection;
      break;
    case MotionSM_Detection:
      if(digitalRead(10) == HIGH){
        isMotion = true;
        MotionSM_State = MotionSM_SetResult;
      }else{
        MotionSM_State = MotionSM_Detection;
      }
      break;
    case MotionSM_SetResult:
      if(digitalRead(10) == LOW){
        Serial.println("Motion Ended.");
        isMotion = false;
        MotionSM_State = MotionSM_Detection;
      }else{
        MotionSM_State = MotionSM_SetResult;
      }
    default:
      MotionSM_State = MotionSM_Start;
      break;
  }
  // no need for second switch statement
  // since nothing is done within the states
  return MotionSM_State;
}

int TickFct_LEDSM(int LEDSM_State){
  static unsigned timer = 0;
  switch(LEDSM_State){
    case LEDSM_Start:
      LEDSM_State = LEDSM_Start;
      if(isMotion){
        timer = 50;
      }
      break;
    default:
      LEDSM_State = LEDSM_Start;
      break;
  }
  switch(LEDSM_State){
    case LEDSM_Start:
      if(timer <= 0 || occupancy == 0){
        digitalWrite(9, LOW);
      }else{
        digitalWrite(9, HIGH);
        timer--;
      }
      break;
    default:
      break;
  }
  return LEDSM_State;
}

int TickFct_BBSM(int BBSM_State){
  switch (BBSM_State){
    case BBSM_Start:
      if(digitalRead(11) == LOW && digitalRead(12) == HIGH){
        Serial.println("BB1 triggered!_BBSM_Start");
        BBSM_State = BBSM_BB1;
      }else if(digitalRead(11) == HIGH && digitalRead(12) == LOW){
        Serial.println("BB2 triggered!_BBSM_Start");
        BBSM_State = BBSM_BB2_;
      }else{
        BBSM_State = BBSM_Start;
      }
      break;
    case BBSM_BB1: //BB1 is triggered first
      if(digitalRead(12) == HIGH){
        BBSM_State = BBSM_BB1;
      }else{
        Serial.println("BB2 triggered!_BBSM_BB1");
        msgQ.enqueue(1);
        BBSM_State = BBSM_BB2;
      }
      break;
    case BBSM_BB2:
      if(digitalRead(12) == LOW){
        BBSM_State = BBSM_BB2;
      }else{
        BBSM_State = BBSM_Start;
      }
      break;
    case BBSM_BB2_: //BB2 is triggered first
      Serial.println("In BBSM_BB2_");
      if(digitalRead(11) == HIGH){
        BBSM_State = BBSM_BB2_;
      }else{
        msgQ.enqueue(-1);
        BBSM_State = BBSM_BB1_;
      }
      break;
    case BBSM_BB1_:
      if(digitalRead(11) == LOW){
        BBSM_State = BBSM_BB1_;
      }else{
        BBSM_State = BBSM_Start;
      }
      break;
    default:
      BBSM_State = BBSM_Start;
      break;
  }
  //no need for second switch statement
  //since nothing is done within the states
  return BBSM_State;
}

/****
 * TODO:
 * - Fine tune motion sensor
*/