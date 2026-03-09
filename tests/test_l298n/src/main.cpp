#include <Arduino.h>

// ── L298N Pin Definitions ──
#define IN1 D1    // Left  forward
#define IN2 D2    // Left  backward
#define IN3 D3    // Right forward
#define IN4 D4    // Right backward
#define ENA D5    // Left  speed PWM
#define ENB D6    // Right speed PWM

// ── Speed Settings ──
#define SPEED_SLOW   100   // 0-255
#define SPEED_MEDIUM 180
#define SPEED_FAST   255

// ── Motor Control Functions ──
void setMotors(bool l_fwd, bool l_bck,
               bool r_fwd, bool r_bck,
               int l_spd, int r_spd) {
  digitalWrite(IN1, l_fwd);
  digitalWrite(IN2, l_bck);
  digitalWrite(IN3, r_fwd);
  digitalWrite(IN4, r_bck);
  analogWrite(ENA, l_spd);
  analogWrite(ENB, r_spd);
}

void stopMotors() {
  setMotors(0, 0, 0, 0, 0, 0);
  Serial.println("⏹  STOP");
}

void moveForward(int speed) {
  setMotors(1, 0, 1, 0, speed, speed);
  Serial.printf("⬆  FORWARD  speed=%d\n", speed);
}

void moveBackward(int speed) {
  setMotors(0, 1, 0, 1, speed, speed);
  Serial.printf("  BACKWARD speed=%d\n", speed);
}

void turnLeft(int speed) {
  setMotors(0, 1, 1, 0, speed, speed);
  Serial.printf("⬅  LEFT     speed=%d\n", speed);
}

void turnRight(int speed) {
  setMotors(1, 0, 0, 1, speed, speed);
  Serial.printf("  RIGHT    speed=%d\n", speed);
}

void spinLeft(int speed) {
  setMotors(0, 0, 1, 0, 0, speed);
  Serial.printf("↺  SPIN LEFT  speed=%d\n", speed);
}

void spinRight(int speed) {
  setMotors(1, 0, 0, 0, speed, 0);
  Serial.printf("↻  SPIN RIGHT speed=%d\n", speed);
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Set all pins as output
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);

  stopMotors();

  Serial.println("\n--- CAR1 MOTOR TEST ---");
  Serial.println("Left  Motors → OUT1 OUT2");
  Serial.println("Right Motors → OUT3 OUT4");
  Serial.println("ENA → D5   ENB → D6");
  Serial.println("IN1→D1 IN2→D2 IN3→D3 IN4→D4");
  Serial.println("───────────────────────────────\n");
  delay(2000);
}

void loop() {

  // ── Test 1: Forward ──
  Serial.println("── Test 1: FORWARD ──");
  moveForward(SPEED_MEDIUM);
  delay(2000);
  stopMotors();
  delay(1000);

  // ── Test 2: Backward ──
  Serial.println("── Test 2: BACKWARD ──");
  moveBackward(SPEED_MEDIUM);
  delay(2000);
  stopMotors();
  delay(1000);

  // ── Test 3: Turn Left ──
  Serial.println("── Test 3: TURN LEFT ──");
  turnLeft(SPEED_MEDIUM);
  delay(1500);
  stopMotors();
  delay(1000);

  // ── Test 4: Turn Right ──
  Serial.println("── Test 4: TURN RIGHT ──");
  turnRight(SPEED_MEDIUM);
  delay(1500);
  stopMotors();
  delay(1000);

  // ── Test 5: Speed Test ──
  Serial.println("── Test 5: SPEED TEST ──");
  Serial.println("SLOW...");
  moveForward(SPEED_SLOW);
  delay(2000);
  Serial.println("MEDIUM...");
  moveForward(SPEED_MEDIUM);
  delay(2000);
  Serial.println("FAST...");
  moveForward(SPEED_FAST);
  delay(2000);
  stopMotors();
  delay(1000);

  // ── Test 6: Spin Test ──
  Serial.println("── Test 6: SPIN TEST ──");
  spinLeft(SPEED_MEDIUM);
  delay(1500);
  stopMotors();
  delay(500);
  spinRight(SPEED_MEDIUM);
  delay(1500);
  stopMotors();
  delay(1000);

  Serial.println("\n── All tests complete! ──\n");
  delay(3000);
}
