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
bool isNoise = false;
bool isOccupied = false;
bool isMotion = false;
Queue<int> msgQ;
Queue<int> msgQRPi;

enum MicSM_States {MicSM_Start, MicSM_Detection, MicSM_SetResult};
enum MotionSM_States {MotionSM_Start, MotionSM_Detection, MotionSM_SetResult};

void TickFct_MicSM(int MicSM_State){
  static unsigned int maxNoise;
  static unsigned int minNoise;
  static unsigned i;
  switch(MicSM_State){
    case MicSM_Start:
      MicSM_State = MicSM_Detection;
      maxNoise = 0;
      minNoise = 1024;
      i = 0;
      break;
    case MicSM_Detection:
      if(i >= 50){
        MicSM_State = MicSM_SetResult;
      }else{
        MicSM_State = MicSM_Detection;
      }
      break;
    case MicSM_SetResult:
      MicSM_State = MicSM_Detection;
      maxNoise = 0;
      minNoise = 1024;
      break;
    default:
      MicSM_State = MicSM_Start;
      break;
  }
  switch(MicSM_State){
    case MicSM_Detection:
      unsigned int sample = analogRead(A0);
      if(sample > maxNoise){
        maxNoise = sample;
      }else if(sample < minNoise){
        minNoise = sample;
      }
      i++;
      break;
    case MicSM_SetResult:
      unsigned int noise = ((maxNoise - minNoise) * 5) / 1024;
      if(noise){
        isNoise = true;
      }
      break;
  }
}

void TickFct_MotionSM(int MotionSM_State){
  switch(MotionSM_State){
    case MotionSM_Start:
      MotionSM_State = MotionSM_Detection;
      break;
    case MotionSM_Detection:
      if(digitalRead(10) == HIGH){
        isNoise = true;
        MotionSM_State = MotionSM_SetResult;
      }else{
        MotionSM_State = MotionSM_Detection;
      }
      break;
    case MotionSM_SetResult:
      if(digitalRead(10) == LOW){
        isNoise = false;
        MotionSM_State = MotionSM_Detection;
      }else{
        MotionSM_State = MotionSM_SetResult;
      }
    default:
      MotionSM_State = MotionSM_Start;
      break;
  }
  switch(MotionSM_State){
    case MotionSM_Detection:
      break;
    case MotionSM_SetResult:
      break;
    default:
      break;
  }
}

void setup() {
  
  Serial.begin(9600); //for serial debugging
  pinMode(A0, INPUT); //for microphone
  pinMode(9, OUTPUT); //for LED
  pinMode(10, INPUT); //for PIR Motion Sensor
  digitalWrite(9, LOW);
  
  LCD.begin(); //LCD Setup
  LCD.setBacklight(HIGH);
  LCD.setCursor(0, 0);
  LCD.print("Hello World!");
}

void loop() {

}
