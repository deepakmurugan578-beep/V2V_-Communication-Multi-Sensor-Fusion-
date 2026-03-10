#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266WiFi.h>
#include <espnow.h>

// ════════════════════════════════════════
//  PIN DEFINITIONS
// ════════════════════════════════════════
#define SCL_PIN      D1    // GPIO5
#define SDA_PIN      D2    // GPIO4
#define IN1_PIN      D3    // GPIO0
#define IN2_PIN      D4    // GPIO2
#define IN3_PIN      D5    // GPIO14
#define IN4_PIN      D6    // GPIO12
#define ENA_PIN      D7    // GPIO13
#define ENB_PIN      D8    // GPIO15
#define IR_FL_PIN    D0    // GPIO16 — IR Front Left
// A0 = IR Front Right — analog pin read as digital
// No boot conflict! No INPUT_PULLUP needed!
#define BUZZER_PIN   1     // GPIO1 = TX pin

// A0 threshold — IR sensor output is digital 0/1
// analogRead(A0) < threshold = LOW = obstacle
#define A0_THRESHOLD 512

// ════════════════════════════════════════
//  CONSTANTS
// ════════════════════════════════════════
#define SPEED_STOP       0
#define SPEED_SLOW       80
#define SPEED_MEDIUM     160
#define SPEED_FAST       220
#define SCREEN_WIDTH     128
#define SCREEN_HEIGHT    64
#define CRASH_THRESHOLD  12000
#define CONN_TIMEOUT     2000
#define ALERT_DURATION   1500
#define LOOP_DELAY       100

// ════════════════════════════════════════
//  OLED
// ════════════════════════════════════════
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT,
                          &Wire, -1);

// ════════════════════════════════════════
//  ESP-NOW PACKET — must match Car1!
// ════════════════════════════════════════
struct V2VData {
  uint8_t  vehicle_id;
  uint8_t  msg_type;        // 1=normal 2=warning 3=emergency
  uint32_t timestamp;
  int16_t  ax, ay, az;
  int16_t  gx, gy, gz;
  float    tilt_angle;
  bool     crash_detected;
  uint16_t distance_cm;
  uint8_t  distance_zone;   // 0=safe 1=warn 2=danger 3=critical
  bool     obstacle_near;
  uint8_t  speed;
  int8_t   direction;       // -1=left 0=fwd 1=right
  bool     braking;
  bool     reversing;
  bool     platoon_active;
  bool     emergency;
};

V2VData       car1Data;
unsigned long lastReceived = 0;
bool          dataReceived = false;

// ════════════════════════════════════════
//  CAR2 LOCAL STATE
// ════════════════════════════════════════
bool     ir_fl        = false;   // IR Front Left  (D0)
bool     ir_fr        = false;   // IR Front Right (A0)
float    car2_tilt    = 0.0;
int16_t  car2_ax      = 0;
int16_t  car2_ay      = 0;
int16_t  car2_az      = 0;
bool     car2_crash   = false;
uint8_t  car2_spd     = 0;
int8_t   car2_dir     = 0;
bool     car2_braking = false;
bool     car2_emerg   = false;

bool     mpu_ok       = false;
bool     oled_ok      = false;

// OLED alert state
String        alertMsg  = "";
unsigned long alertTime = 0;

// ════════════════════════════════════════
//  MPU6050 — DIRECT I2C (no Adafruit lib)
// ════════════════════════════════════════
#define MPU_ADDR 0x68

void mpuWrite(uint8_t reg, uint8_t val) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(reg);
  Wire.write(val);
  Wire.endTransmission(true);
}

int16_t mpuReadWord(uint8_t reg) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom((uint8_t)MPU_ADDR,
                   (uint8_t)2,
                   (uint8_t)true);
  if (Wire.available() < 2) return 0;
  return (int16_t)((Wire.read() << 8) | Wire.read());
}

bool mpuBegin() {
  mpuWrite(0x6B, 0x00);  // wake up
  delay(100);
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x75);      // WHO_AM_I
  Wire.endTransmission(false);
  Wire.requestFrom((uint8_t)MPU_ADDR,
                   (uint8_t)1,
                   (uint8_t)true);
  if (!Wire.available()) return false;
  uint8_t id = Wire.read();
  return (id == 0x68 || id == 0x70 || id == 0x72);
}

void readMPU() {
  car2_ax = mpuReadWord(0x3B);
  car2_ay = mpuReadWord(0x3D);
  car2_az = mpuReadWord(0x3F);
  float ax_f = (float)car2_ax / 16384.0f;
  float ay_f = (float)car2_ay / 16384.0f;
  float az_f = (float)car2_az / 16384.0f;
  car2_tilt  = atan2f(ax_f,
               sqrtf(ay_f * ay_f + az_f * az_f))
               * 180.0f / (float)PI;
  car2_crash = (abs(car2_ax) > CRASH_THRESHOLD ||
                abs(car2_ay) > CRASH_THRESHOLD);
}

// ════════════════════════════════════════
//  IR SENSORS
// ════════════════════════════════════════
void readIR() {
  // D0 — normal digital read
  ir_fl = (digitalRead(IR_FL_PIN) == LOW);

  // A0 — analog pin read as digital
  // IR sensor output: LOW = obstacle detected
  // analogRead < 512 means LOW (obstacle)
  ir_fr = (analogRead(A0) < A0_THRESHOLD);
}

// ════════════════════════════════════════
//  BUZZER — non-blocking timer
// ════════════════════════════════════════
unsigned long buzzEnd    = 0;
bool          buzzActive = false;

void buzzStart(unsigned long ms) {
  digitalWrite(BUZZER_PIN, HIGH);
  buzzActive = true;
  buzzEnd    = millis() + ms;
}

void buzzUpdate() {
  if (buzzActive && millis() >= buzzEnd) {
    digitalWrite(BUZZER_PIN, LOW);
    buzzActive = false;
  }
}

void beepShort()  { if (!buzzActive) buzzStart(150); }
void beepMedium() { if (!buzzActive) buzzStart(400); }
void beepLong()   { if (!buzzActive) buzzStart(900); }

// ════════════════════════════════════════
//  MOTORS
// ════════════════════════════════════════
void stopMotors() {
  digitalWrite(IN1_PIN, LOW);
  digitalWrite(IN2_PIN, LOW);
  digitalWrite(IN3_PIN, LOW);
  digitalWrite(IN4_PIN, LOW);
  analogWrite(ENA_PIN, 0);
  analogWrite(ENB_PIN, 0);
  car2_spd     = SPEED_STOP;
  car2_dir     = 0;
  car2_braking = true;
}

void driveMotors(bool lf, bool lb,
                 bool rf, bool rb,
                 int  ls, int  rs) {
  digitalWrite(IN1_PIN, lf);
  digitalWrite(IN2_PIN, lb);
  digitalWrite(IN3_PIN, rf);
  digitalWrite(IN4_PIN, rb);
  analogWrite(ENA_PIN, ls);
  analogWrite(ENB_PIN, rs);
  car2_braking = false;
}

void moveForward(int spd) {
  driveMotors(1, 0, 1, 0, spd, spd);
  car2_spd = spd;
  car2_dir = 0;
}

void turnLeft(int spd) {
  driveMotors(1, 0, 1, 0, spd / 2, spd);
  car2_spd = spd;
  car2_dir = -1;
}

void turnRight(int spd) {
  driveMotors(1, 0, 1, 0, spd, spd / 2);
  car2_spd = spd;
  car2_dir = 1;
}

// ════════════════════════════════════════
//  OLED
// ════════════════════════════════════════
void showAlert(const char* msg) {
  alertMsg  = String(msg);
  alertTime = millis();
}

void updateOLED() {
  if (!oled_ok) return;
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  // Alert overlay
  if (alertMsg.length() > 0 &&
      (millis() - alertTime) <
      (unsigned long)ALERT_DURATION) {
    display.setTextSize(2);
    display.setCursor(0, 18);
    display.println(alertMsg);
    display.display();
    return;
  }
  alertMsg = "";

  bool linked = ((millis() - lastReceived) <
                 (unsigned long)CONN_TIMEOUT);
  display.setTextSize(1);

  // Header
  display.setCursor(0, 0);
  display.print("V2V CAR2");
  display.setCursor(72, 0);
  display.println(linked ? "[FOLLOW]" : "[WAIT..]");
  display.drawLine(0, 9, 128, 9, SSD1306_WHITE);

  // Car1 data
  display.setCursor(0, 12);
  if (linked) {
    display.printf("C1 D:%3dcm Z:%d S:%d",
      car1Data.distance_cm,
      car1Data.distance_zone,
      car1Data.speed);
  } else {
    display.print("C1: NO SIGNAL    ");
  }

  // Car1 status
  display.setCursor(0, 22);
  if (!linked)
    display.print("C1: WAITING...    ");
  else if (car1Data.emergency)
    display.print("C1: !! EMERGENCY !");
  else if (car1Data.braking)
    display.print("C1: !! BRAKING !! ");
  else if (car1Data.platoon_active)
    display.print("C1: PLATOON ACTIVE");
  else
    display.print("C1: RUNNING       ");

  // IR sensors
  display.setCursor(0, 33);
  display.printf("IR FL:%-3s  FR:%-3s",
    ir_fl ? "OBS" : "CLR",
    ir_fr ? "OBS" : "CLR");

  // Tilt + speed
  display.setCursor(0, 43);
  display.printf("TILT:%5.1f  SPD:%3d",
    car2_tilt, car2_spd);

  // Footer status
  display.drawLine(0, 53, 128, 53, SSD1306_WHITE);
  display.setCursor(0, 56);
  if      (car2_crash)
    display.print("!! CRASH DETECTED !!");
  else if (car2_emerg)
    display.print("!! EMERGENCY STOP !!");
  else if (!linked)
    display.print(">> WAITING FOR CAR1 ");
  else if (car1Data.emergency)
    display.print("!! CAR1 EMERGENCY !");
  else if (car1Data.braking)
    display.print("!! CAR1 BRAKING !!  ");
  else if (car1Data.platoon_active)
    display.print(">> FOLLOWING CAR1   ");
  else
    display.print(">> STANDBY          ");

  display.display();
}

// ════════════════════════════════════════
//  SERIAL STUDIO OUTPUT
//  /*LINK,C1DIST,C1ZONE,C1SPD,FL,FR,
//    TILT,C2SPD,CRASH,EMRG,BRAKE*/
// ════════════════════════════════════════
void serialStudioOutput() {
  bool linked = ((millis() - lastReceived) <
                 (unsigned long)CONN_TIMEOUT);
  Serial.printf(
    "/*%d,%d,%d,%d,%d,%d,%.1f,%d,%d,%d,%d*/\n",
    (int)linked,
    (int)car1Data.distance_cm,
    (int)car1Data.distance_zone,
    (int)car1Data.speed,
    (int)ir_fl,
    (int)ir_fr,
    car2_tilt,
    (int)car2_spd,
    (int)car2_crash,
    (int)car1Data.emergency,
    (int)car1Data.braking
  );
}

// ════════════════════════════════════════
//  ESP-NOW RECEIVE
// ════════════════════════════════════════
void onDataReceived(uint8_t *mac,
                    uint8_t *inData,
                    uint8_t  len) {
  if (len == sizeof(V2VData)) {
    memcpy(&car1Data, inData, sizeof(V2VData));
    dataReceived = true;
    lastReceived = millis();
  }
}

// ════════════════════════════════════════
//  FOLLOW LOGIC
// ════════════════════════════════════════
void handleFollowLogic() {
  bool linked = ((millis() - lastReceived) <
                 (unsigned long)CONN_TIMEOUT);
  car2_emerg = false;

  // 1 — Car2 crash
  if (car2_crash) {
    stopMotors();
    car2_emerg = true;
    beepLong();
    showAlert("CAR2 CRASH!");
    return;
  }

  // 2 — Both front IR blocked
  if (ir_fl && ir_fr) {
    stopMotors();
    beepMedium();
    showAlert("BLOCKED!");
    return;
  }

  // 3 — Left IR → steer right
  if (ir_fl) {
    turnRight(SPEED_MEDIUM);
    beepShort();
    return;
  }

  // 4 — Right IR → steer left
  if (ir_fr) {
    turnLeft(SPEED_MEDIUM);
    beepShort();
    return;
  }

  // 5 — No Car1 signal
  if (!linked) {
    stopMotors();
    return;
  }

  // 6 — Car1 emergency or crash
  if (car1Data.emergency || car1Data.crash_detected) {
    stopMotors();
    car2_emerg = true;
    beepLong();
    showAlert("C1 EMERG!");
    return;
  }

  // 7 — Car1 braking
  if (car1Data.braking) {
    stopMotors();
    beepMedium();
    showAlert("C1 BRAKE!");
    return;
  }

  // 8 — Mirror Car1 direction
  if (car1Data.direction == -1) {
    turnLeft(car1Data.speed);
    return;
  }
  if (car1Data.direction == 1) {
    turnRight(car1Data.speed);
    return;
  }

  // 9 — Mirror Car1 speed zone
  switch (car1Data.distance_zone) {
    case 3: stopMotors();                  break;
    case 2: moveForward(SPEED_SLOW);       break;
    case 1: moveForward(SPEED_MEDIUM);     break;
    default: moveForward(SPEED_FAST);      break;
  }
}

// ════════════════════════════════════════
//  SETUP
// ════════════════════════════════════════
void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println();
  Serial.println("=== CAR2 FOLLOWER BOOTING ===");

  // Motor pins
  pinMode(IN1_PIN, OUTPUT);
  pinMode(IN2_PIN, OUTPUT);
  pinMode(IN3_PIN, OUTPUT);
  pinMode(IN4_PIN, OUTPUT);
  pinMode(ENA_PIN, OUTPUT);
  pinMode(ENB_PIN, OUTPUT);
  stopMotors();

  // IR Front Left — normal digital pin
  pinMode(IR_FL_PIN, INPUT_PULLUP);
  // IR Front Right — A0 analog pin
  // No pinMode needed for analogRead on A0

  // I2C
  pinMode(SDA_PIN, INPUT_PULLUP);
  pinMode(SCL_PIN, INPUT_PULLUP);
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(100000);
  delay(100);

  // OLED
  if (display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    oled_ok = true;
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("CAR2 BOOTING...");
    display.display();
    Serial.println("OLED: OK");
  } else {
    Serial.println("OLED: FAILED");
  }

  // MPU6050
  if (mpuBegin()) {
    mpu_ok = true;
    Serial.println("MPU6050: OK");
    if (oled_ok) {
      display.setCursor(0, 12);
      display.println("MPU6050: OK");
      display.display();
    }
  } else {
    Serial.println("MPU6050: FAILED");
  }

  // ESP-NOW
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  wifi_set_channel(1);
  if (esp_now_init() == 0) {
    esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
    esp_now_register_recv_cb(onDataReceived);
    Serial.println("ESP-NOW: OK");
    if (oled_ok) {
      display.setCursor(0, 24);
      display.println("ESP-NOW: OK");
      display.display();
    }
  } else {
    Serial.println("ESP-NOW: FAILED");
  }

  // Safe defaults for car1Data
  memset(&car1Data, 0, sizeof(V2VData));
  car1Data.distance_cm   = 999;
  car1Data.distance_zone = 0;

  delay(800);

  // Ready screen
  if (oled_ok) {
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(28, 12);
    display.println("CAR2");
    display.setCursor(16, 36);
    display.println("READY!");
    display.display();
  }

  Serial.println("IR Front Left  : D0");
  Serial.println("IR Front Right : A0 (analog as digital)");
  Serial.println("Buzzer         : TX (GPIO1)");
  Serial.println("=== CAR2 READY ===");
  delay(500);

  // Stop Serial to free TX pin for buzzer
  Serial.flush();
  Serial.end();

  // Now TX is free — setup buzzer
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  // Double startup beep
  digitalWrite(BUZZER_PIN, HIGH); delay(150);
  digitalWrite(BUZZER_PIN, LOW);  delay(100);
  digitalWrite(BUZZER_PIN, HIGH); delay(150);
  digitalWrite(BUZZER_PIN, LOW);

  // Restart Serial for dashboard output
  Serial.begin(115200);
  delay(100);
}

// ════════════════════════════════════════
//  LOOP
// ════════════════════════════════════════
void loop() {

  // 1. Read MPU6050
  if (mpu_ok) readMPU();

  // 2. Read IR sensors
  readIR();

  // 3. Buzzer timer update
  buzzUpdate();

  // 4. Follow logic
  handleFollowLogic();

  // 5. OLED update
  updateOLED();

  // 6. Serial Studio output
  serialStudioOutput();

  delay(LOOP_DELAY);
}