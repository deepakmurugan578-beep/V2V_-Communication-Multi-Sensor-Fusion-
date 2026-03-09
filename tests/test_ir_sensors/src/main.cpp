#include <Arduino.h>

#define IR_FRONT_LEFT  D5
#define IR_FRONT_RIGHT D6
#define IR_REAR_LEFT   D7
#define IR_REAR_RIGHT  D8

bool fl, fr, rl, rr;

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(IR_FRONT_LEFT,  INPUT);
  pinMode(IR_FRONT_RIGHT, INPUT);
  pinMode(IR_REAR_LEFT,   INPUT);
  pinMode(IR_REAR_RIGHT,  INPUT);

  Serial.println("\n--- 4x IR SENSOR TEST ---");
  Serial.println("FL→D5  FR→D6  RL→D7  RR→D8");
  Serial.println("─────────────────────────────\n");
}

void readSensors() {
  fl = (digitalRead(IR_FRONT_LEFT)  == LOW);
  fr = (digitalRead(IR_FRONT_RIGHT) == LOW);
  rl = (digitalRead(IR_REAR_LEFT)   == LOW);
  rr = (digitalRead(IR_REAR_RIGHT)  == LOW);
}

void printStatus() {
  Serial.println("┌──────────────────────────────┐");
  Serial.printf ("│ FRONT LEFT  : %s │\n", fl ? " DETECTED  " : " clear     ");
  Serial.printf ("│ FRONT RIGHT : %s │\n", fr ? " DETECTED  " : " clear     ");
  Serial.printf ("│ REAR LEFT   : %s │\n", rl ? " DETECTED  " : " clear     ");
  Serial.printf ("│ REAR RIGHT  : %s │\n", rr ? " DETECTED  " : " clear     ");
  Serial.println("├──────────────────────────────┤");

  Serial.print  ("│ STATUS : ");
  if (!fl && !fr && !rl && !rr)
    Serial.println(" ALL CLEAR          │");
  else if (fl && fr)
    Serial.println("FRONT BLOCKED      │");
  else if (fl)
    Serial.println("  FRONT LEFT OBS    │");
  else if (fr)
    Serial.println(" FRONT RIGHT OBS   │");
  else if (rl && rr)
    Serial.println(" REAR BLOCKED       │");
  else if (rl)
    Serial.println("  REAR LEFT OBS     │");
  else if (rr)
    Serial.println("  REAR RIGHT OBS    │");

  Serial.println("└──────────────────────────────┘\n");
}

void loop() {
  readSensors();
  printStatus();
  delay(500);
}
