#include <Arduino.h>

#define BUZZER_PIN D8

// ── Active buzzer functions ──
void buzzOn()  { digitalWrite(BUZZER_PIN, HIGH); }
void buzzOff() { digitalWrite(BUZZER_PIN, LOW);  }

// ── Passive buzzer tone ──
void buzzTone(int freq, int duration) {
  tone(BUZZER_PIN, freq, duration);
  delay(duration);
  noTone(BUZZER_PIN);
}

// ── Sound patterns ──
void beepSingle() {
  buzzOn(); delay(200);
  buzzOff(); delay(200);
}

void beepDouble() {
  buzzOn(); delay(150);
  buzzOff(); delay(100);
  buzzOn(); delay(150);
  buzzOff(); delay(200);
}

void beepTriple() {
  for (int i = 0; i < 3; i++) {
    buzzOn(); delay(100);
    buzzOff(); delay(100);
  }
  delay(200);
}

void beepFast(int times) {
  for (int i = 0; i < times; i++) {
    buzzOn(); delay(80);
    buzzOff(); delay(80);
  }
}

void beepContinuous(int duration) {
  buzzOn();
  delay(duration);
  buzzOff();
}

void beepSOS() {
  // S = 3 short
  for (int i = 0; i < 3; i++) {
    buzzOn(); delay(150);
    buzzOff(); delay(150);
  }
  delay(200);
  // O = 3 long
  for (int i = 0; i < 3; i++) {
    buzzOn(); delay(400);
    buzzOff(); delay(150);
  }
  delay(200);
  // S = 3 short
  for (int i = 0; i < 3; i++) {
    buzzOn(); delay(150);
    buzzOff(); delay(150);
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  pinMode(BUZZER_PIN, OUTPUT);
  buzzOff();

  Serial.println("\n--- BUZZER TEST ---");
  Serial.println("Buzzer (+) → D8");
  Serial.println("Buzzer (-) → GND");
  Serial.println("───────────────────\n");

  // Startup beep
  beepTriple();
  Serial.println(" Startup beep done!");
  delay(1000);
}

void loop() {

  // ── Test V2V Sound Patterns ──
  Serial.println("1️  Single beep → data received");
  beepSingle();
  delay(1000);

  Serial.println("2️  Double beep → obstacle detected");
  beepDouble();
  delay(1000);

  Serial.println("3️  Triple beep → system startup");
  beepTriple();
  delay(1000);

  Serial.println("4️  Fast beep → collision warning");
  beepFast(6);
  delay(1000);

  Serial.println("5️ Continuous → emergency/crash");
  beepContinuous(1500);
  delay(1000);

  Serial.println("6️ SOS pattern → critical alert");
  beepSOS();
  delay(1000);

  Serial.println("\n── Cycle complete ──\n");
  delay(2000);
}
