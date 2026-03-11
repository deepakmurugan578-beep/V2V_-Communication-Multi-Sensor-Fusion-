#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266WiFi.h>
#include <espnow.h>


#define SCL_PIN   D1
#define SDA_PIN   D2
#define IN1       D3
#define IN2       D4
#define ECHO_PIN  D5
#define TRIG_PIN  D6
#define IN3       D7
#define IN4       D8
#define ENA       D0
#define ENB       3     // RX = GPIO3


#define SPEED_STOP    0
#define SPEED_SLOW    80
#define SPEED_MEDIUM  160
#define SPEED_FAST    220

#define DIST_CRITICAL  11
#define DIST_DANGER    20
#define DIST_WARNING   50

#define CRASH_THRESHOLD  12000
#define TILT_THRESHOLD   30.0f

#define SCREEN_WIDTH   128
#define SCREEN_HEIGHT   64
#define ALERT_DURATION 1500
#define CAR2_TIMEOUT   2000   // ms before Car2 = lost


uint8_t car2MAC[] = {0xC8, 0xC9, 0xA3, 0x66, 0x97, 0xD1};


Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT,
                          &Wire, -1);



// Car1 → Car2 packet
struct Car1Data {
  uint8_t  vehicle_id;      // always 1
  uint8_t  msg_type;        // 1=normal 2=warn 3=emergency
  uint32_t timestamp;
  // MPU6050
  int16_t  ax, ay, az;
  int16_t  gx, gy, gz;
  float    tilt_angle;
  bool     crash_detected;
  // HC-SR04
  uint16_t distance_cm;
  uint8_t  distance_zone;   // 0=safe 1=warn 2=danger 3=critical
  bool     obstacle_near;
  // Motors
  uint8_t  speed;
  int8_t   direction;       // -1=left 0=fwd 1=right
  bool     braking;
  bool     reversing;
  // Flags
  bool     platoon_active;
  bool     emergency;
  // Scenario
  uint8_t  active_scenario; // 1-5
};

// Car2 → Car1 packet (received)
struct Car2Data {
  uint8_t  vehicle_id;      // always 2
  uint8_t  msg_type;
  uint32_t timestamp;
  // MPU6050
  int16_t  ax, ay, az;
  float    tilt_angle;
  bool     crash_detected;
  // IR sensor
  bool     ir_obstacle;     // IR detected obstacle
  // Motors
  uint8_t  speed;
  int8_t   direction;
  bool     braking;
  // Flags
  bool     platoon_active;
  bool     emergency;
  // Scenario
  uint8_t  active_scenario;
};

Car1Data  txData;           // data we SEND to Car2
Car2Data  car2Data;         // data we RECEIVE from Car2

bool          espnow_ok     = false;
bool          mpu_ok        = false;
bool          oled_ok       = false;
unsigned long car2LastSeen  = 0;
bool          car2Linked    = false;


#define MPU_ADDR 0x68

void mpuWrite(uint8_t reg, uint8_t val) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(reg); Wire.write(val);
  Wire.endTransmission(true);
}

int16_t mpuReadWord(uint8_t reg) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom((uint8_t)MPU_ADDR,
                   (uint8_t)2, (uint8_t)true);
  if (Wire.available() < 2) return 0;
  return (int16_t)((Wire.read() << 8) | Wire.read());
}

bool mpuBegin() {
  mpuWrite(0x6B, 0x00);
  delay(100);
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x75);
  Wire.endTransmission(false);
  Wire.requestFrom((uint8_t)MPU_ADDR,
                   (uint8_t)1, (uint8_t)true);
  if (!Wire.available()) return false;
  uint8_t id = Wire.read();
  return (id == 0x68 || id == 0x70 || id == 0x72);
}

void readMPU() {
  txData.ax = mpuReadWord(0x3B);
  txData.ay = mpuReadWord(0x3D);
  txData.az = mpuReadWord(0x3F);
  txData.gx = mpuReadWord(0x43);
  txData.gy = mpuReadWord(0x45);
  txData.gz = mpuReadWord(0x47);

  float ax_f = (float)txData.ax / 16384.0f;
  float ay_f = (float)txData.ay / 16384.0f;
  float az_f = (float)txData.az / 16384.0f;
  txData.tilt_angle = atan2f(ax_f,
    sqrtf(ay_f * ay_f + az_f * az_f))
    * 180.0f / (float)PI;
}


float readDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long dur = pulseIn(ECHO_PIN, HIGH, 30000);
  if (dur == 0) return -1;
  return (float)(dur * 0.034f / 2.0f);
}

uint8_t getZone(float cm) {
  if (cm <= 0)             return 0;
  if (cm <= DIST_CRITICAL) return 3;
  if (cm <= DIST_DANGER)   return 2;
  if (cm <= DIST_WARNING)  return 1;
  return 0;
}


void stopMotors() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
  analogWrite(ENA, 0);    analogWrite(ENB, 0);
  txData.speed     = SPEED_STOP;
  txData.direction = 0;
  txData.braking   = true;
}

void driveMotors(bool lf, bool lb,
                 bool rf, bool rb,
                 int  ls, int  rs) {
  digitalWrite(IN1, lf); digitalWrite(IN2, lb);
  digitalWrite(IN3, rf); digitalWrite(IN4, rb);
  analogWrite(ENA, ls);  analogWrite(ENB, rs);
  txData.braking = false;
}

void moveForward(int spd) {
  driveMotors(1, 0, 1, 0, spd, spd);
  txData.speed = spd; txData.direction = 0;
}

void turnLeft(int spd) {
  driveMotors(1, 0, 1, 0, spd / 2, spd);
  txData.speed = spd; txData.direction = -1;
}

void turnRight(int spd) {
  driveMotors(1, 0, 1, 0, spd, spd / 2);
  txData.speed = spd; txData.direction = 1;
}


String        alertMsg  = "";
unsigned long alertTime = 0;

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
    display.setCursor(0, 16);
    display.println(alertMsg);
    display.display();
    return;
  }
  alertMsg = "";

  display.setTextSize(1);

  // Header
  display.setCursor(0, 0);
  display.print("V2V CAR1");
  display.setCursor(60, 0);
  display.printf(" SC:%d %s",
    txData.active_scenario,
    car2Linked ? "[C2:OK]" : "[C2:--]");
  display.drawLine(0, 9, 128, 9, SSD1306_WHITE);

  // Distance
  display.setCursor(0, 12);
  if (txData.distance_cm == 999)
    display.print("DIST:---cm");
  else
    display.printf("DIST:%3dcm", txData.distance_cm);
  const char* zoneStr[] = {" SAFE"," WARN"," DNGR"," CRIT"};
  display.println(zoneStr[txData.distance_zone]);

  // Tilt
  display.setCursor(0, 22);
  display.printf("TILT:%.1f  SPD:%3d",
    txData.tilt_angle, txData.speed);

  // Car2 status
  display.setCursor(0, 32);
  if (!car2Linked)
    display.print("C2: WAITING...      ");
  else if (car2Data.crash_detected)
    display.print("C2: !! CRASH !!     ");
  else if (car2Data.ir_obstacle)
    display.print("C2: IR OBSTACLE!    ");
  else if (car2Data.emergency)
    display.print("C2: EMERGENCY!      ");
  else
    display.printf("C2: SPD:%3d SC:%d   ",
      car2Data.speed,
      car2Data.active_scenario);

  // Link
  display.setCursor(0, 42);
  display.printf("LINK:%s  MSG:%d",
    espnow_ok ? "OK  " : "WAIT",
    txData.msg_type);

  // Footer
  display.drawLine(0, 53, 128, 53, SSD1306_WHITE);
  display.setCursor(0, 56);
  if      (txData.crash_detected) display.print("!! CAR1 CRASH !!    ");
  else if (car2Data.crash_detected) display.print("!! CAR2 CRASH !!    ");
  else if (txData.emergency)      display.print("!! EMERGENCY STOP !!");
  else if (car2Data.ir_obstacle)  display.print("C2 OBSTACLE-TURNING ");
  else if (txData.platoon_active) display.print(">> PLATOON RUNNING  ");
  else                            display.print(">> STANDBY          ");

  display.display();
}


void onSent(uint8_t *mac, uint8_t status) {
  espnow_ok = (status == 0);
}

void onReceive(uint8_t *mac,
               uint8_t *inData, uint8_t len) {
  if (len == sizeof(Car2Data)) {
    memcpy(&car2Data, inData, sizeof(Car2Data));
    car2LastSeen = millis();
    car2Linked   = true;
  }
}


void handleScenarios() {

  // Reset flags every loop
  txData.emergency      = false;
  txData.crash_detected = false;
  txData.braking        = false;
  txData.reversing      = false;
  txData.platoon_active = false;
  txData.msg_type       = 1;

  // Check Car2 link timeout
  car2Linked = ((millis() - car2LastSeen) <
                (unsigned long)CAR2_TIMEOUT
                && car2LastSeen > 0);

  
  bool car1Crash = (abs(txData.ax) > CRASH_THRESHOLD ||
                    abs(txData.ay) > CRASH_THRESHOLD);
  bool car2Crash = (car2Linked && car2Data.crash_detected);

  if (car1Crash) {
    txData.crash_detected  = true;
    txData.emergency       = true;
    txData.msg_type        = 3;
    txData.active_scenario = 3;
    stopMotors();
    showAlert("CAR1 CRASH!");
    Serial.println("CRASH:CAR1");
    return;
  }

  if (car2Crash) {
    txData.emergency       = true;
    txData.msg_type        = 3;
    txData.active_scenario = 3;
    stopMotors();
    showAlert("CAR2 CRASH!");
    Serial.println("CRASH:CAR2");
    return;
  }

 
  if (txData.distance_zone == 3) {
    txData.emergency       = true;
    txData.braking         = true;
    txData.msg_type        = 3;
    txData.active_scenario = 1;
    stopMotors();
    showAlert("COLLISION!");
    Serial.println("SCENARIO:1 COLLISION");
    return;
  }

  
  if (car2Linked && car2Data.ir_obstacle) {
    txData.msg_type        = 2;
    txData.active_scenario = 4;
    // Mirror Car2 direction
    if (car2Data.direction == 1) {
      turnRight(SPEED_MEDIUM);
      Serial.println("SCENARIO:4 C2_OBS->RIGHT");
    } else {
      turnLeft(SPEED_MEDIUM);
      Serial.println("SCENARIO:4 C2_OBS->LEFT");
    }
    return;
  }

  
  if (txData.distance_zone == 2) {
    txData.msg_type        = 2;
    txData.active_scenario = 2;
    moveForward(SPEED_SLOW);
    Serial.println("SCENARIO:2 DANGER->SLOW");
    return;
  }

  if (txData.distance_zone == 1) {
    txData.msg_type        = 2;
    txData.active_scenario = 2;
    moveForward(SPEED_MEDIUM);
    Serial.println("SCENARIO:2 WARN->MEDIUM");
    return;
  }

  
  txData.platoon_active  = true;
  txData.msg_type        = 1;
  txData.active_scenario = 5;
  moveForward(SPEED_FAST);
  Serial.println("SCENARIO:5 PLATOON");
}


void serialOutput() {
  Serial.printf(
    "/*%d,%d,%.1f,%d,%d,%d,%d,%d,%d,%d*/\n",
    txData.distance_cm,
    txData.distance_zone,
    txData.tilt_angle,
    txData.speed,
    txData.direction,
    txData.msg_type,
    (int)txData.crash_detected,
    (int)txData.braking,
    (int)espnow_ok,
    txData.active_scenario
  );
}


void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println();
  Serial.println("=== CAR1 LEADER BIDIR BOOTING ===");

  // Motor pins
  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT); pinMode(ENB, OUTPUT);
  stopMotors();

  // HC-SR04
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  digitalWrite(TRIG_PIN, LOW);

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
    display.println("CAR1 BIDIR BOOT...");
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

  // ESP-NOW — BIDIRECTIONAL
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  if (esp_now_init() == 0) {
    // Car1 = CONTROLLER + SLAVE (bidirectional!)
    esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
    // Send callback
    esp_now_register_send_cb(onSent);
    // Receive callback — NEW for bidirectional!
    esp_now_register_recv_cb(onReceive);
    // Register Car2 as peer
    esp_now_add_peer(car2MAC,
      ESP_NOW_ROLE_COMBO, 1, NULL, 0);
    Serial.println("ESP-NOW BIDIR: OK");
    if (oled_ok) {
      display.setCursor(0, 24);
      display.println("ESP-NOW BIDIR OK");
      display.display();
    }
  } else {
    Serial.println("ESP-NOW: FAILED");
  }

  // Init data
  memset(&txData,   0, sizeof(Car1Data));
  memset(&car2Data, 0, sizeof(Car2Data));
  txData.vehicle_id  = 1;
  txData.msg_type    = 1;
  txData.distance_cm = 999;

  delay(800);

  if (oled_ok) {
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(28, 12);
    display.println("CAR1");
    display.setCursor(16, 36);
    display.println("READY!");
    display.display();
    delay(1000);
  }

  Serial.println("=== CAR1 BIDIR READY ===");
}

void loop() {

  // 1. Read MPU6050
  if (mpu_ok) readMPU();

  // 2. Read HC-SR04
  float dist = readDistance();
  if (dist < 0 || dist > 400) {
    txData.distance_cm   = 999;
    txData.distance_zone = 0;
    txData.obstacle_near = false;
  } else {
    txData.distance_cm   = (uint16_t)dist;
    txData.distance_zone = getZone(dist);
    txData.obstacle_near = (dist <= DIST_DANGER);
  }

  // 3. Timestamp
  txData.timestamp = millis();

  // 4. Run 5 scenarios
  handleScenarios();

  // 5. Send to Car2 via ESP-NOW
  esp_now_send(car2MAC,
    (uint8_t*)&txData, sizeof(Car1Data));

  // 6. Update OLED
  updateOLED();

  // 7. Serial output
  serialOutput();

  delay(100);
}
