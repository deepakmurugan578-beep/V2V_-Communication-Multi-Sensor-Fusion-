#include <Arduino.h>

#define TRIG_PIN D6
#define ECHO_PIN D5

// Variables
long duration;
float distance_cm;
float distance_mm;

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Make sure TRIG starts LOW
  digitalWrite(TRIG_PIN, LOW);
  delay(100);

  Serial.println("\n--- HC-SR04 TEST ---");
  Serial.println("Wiring check:");
  Serial.println("VCC  → Vin (5V)");
  Serial.println("GND  → GND");
  Serial.println("TRIG → D6  (direct)");
  Serial.println("ECHO → D5  (via 1kΩ+2kΩ divider)");
  Serial.println("──────────────────────────────");
  Serial.println("Point sensor at object 2cm-400cm\n");
}

float readDistance() {
  // Clear TRIG
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  // Send 10µs pulse
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Read ECHO pulse duration
  // Timeout = 30000µs = ~5 meters max
  duration = pulseIn(ECHO_PIN, HIGH, 30000);

  // Convert to distance
  // Speed of sound = 0.034 cm/µs
  // Divide by 2 for round trip
  if (duration == 0) {
    return -1;  // timeout / out of range
  }
  return duration * 0.034 / 2.0;
}

void loop() {
  distance_cm = readDistance();
  distance_mm = distance_cm * 10.0;

  if (distance_cm == -1) {
    Serial.println("  Out of range! (>400cm or no object)");

  } else if (distance_cm < 2) {
    Serial.println("  Too close! (min 2cm)");

  } else {
    // Valid reading
    Serial.printf("📏 Distance: %6.1f mm  |  %5.1f cm  |  %.2f m",
      distance_mm, distance_cm, distance_cm / 100.0);

    // Range indicator
    if      (distance_cm < 10)  Serial.println("   VERY CLOSE");
    else if (distance_cm < 20)  Serial.println("   CLOSE");
    else if (distance_cm < 50)  Serial.println("   NEAR");
    else if (distance_cm < 100) Serial.println("   MEDIUM");
    else if (distance_cm < 200) Serial.println("   FAR");
    else                        Serial.println("   VERY FAR");
  }

  delay(300);
}