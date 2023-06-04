// #include <LiquidCrystal.h>
// #include <string.h>


// int noiseCnt = 0;
// const int rs = 8, en = 7, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
// LiquidCrystal LCD(rs, en, d4, d5, d6, d7);

// void setup() {
  // put your setup code here, to run once:
  // Serial.begin(9600);
  // pinMode(A0, INPUT);
  // pinMode(9, OUTPUT);
  // pinMode(10, INPUT);
  // digitalWrite(9, LOW);
  
  // LCD.begin(16, 2);
  // LCD.print("Noise Detected:");
  // LCD.setCursor(0, 1);
  // LCD.print(String(noiseCnt) + " times.");
// }

// void loop() {
//   if(digitalRead(10) == HIGH){
//     Serial.println("Motion Detected!");
//     digitalWrite(9, HIGH);
//   }else{
//     Serial.println("Motion Ended.");
//     digitalWrite(9, LOW);
//   }
//   delay(500);
  // put your main code here, to run repeatedly:
  // static int timer = -1;
  // unsigned long start = millis();
  // unsigned int max = 0;
  // unsigned int min = 1024;
  // while(millis()-start < 50){
  //   unsigned int sample = analogRead(A0);
  //   if(sample > max){
  //     max = sample;
  //   }else if(sample < min){
  //     min = sample;
  //   }
  // }
  // unsigned int noise = ((max - min) * 5) / 1024;
  // if(noise){
  //   noiseCnt++;
  //   Serial.println("Noise Detected!");
  //   digitalWrite(9, HIGH);
  //   timer = 5000;
  //   LCD.clear();
  //   LCD.begin(16, 2);
  //   LCD.print("Noise Detected:");
  //   LCD.setCursor(0, 1);
  //   LCD.print(String(noiseCnt) + " times.");
  // }
  // if(timer <= 0){
  //   digitalWrite(9, LOW);
  //   timer = 0;
  // }else{
  //   timer -= millis() - start;
  // }
//   // Serial.println();
// }
