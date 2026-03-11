#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266WiFi.h>
#include <espnow.h>

#define SCL_PIN      D1    
#define SDA_PIN      D2    
#define IN1_PIN      D3    
#define IN2_PIN      D4    
#define IN3_PIN      D5    
#define IN4_PIN      D6    
#define ENA_PIN      D7    
#define ENB_PIN      D8    
#define IR_PIN       D0    
#define BUZZER_PIN   1     


#define SPEED_STOP       0
#define SPEED_SLOW       80
#define SPEED_MEDIUM     160
#define SPEED_FAST       220

#define CRASH_THRESHOLD  12000
#define SCREEN_WIDTH     128
#define SCREEN_HEIGHT    64
#define ALERT_DURATION   1500
#define CAR1_TIMEOUT     2000
#define LOOP_DELAY       100
#define SEND_INTERVAL    100  // ms between Car2→Car1 sends


uint8_t car1MAC[] = {0xC4, 0x5B, 0xBE, 0x71, 0x31, 0x47};


Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT,
                          &Wire, -1);

struct Car1Data {
  uint8_t  vehicle_id;
  uint8_t  msg_type;
  uint32_t timestamp;
  int16_t  ax, ay, az;
  int16_t  gx, gy, gz;
  float    tilt_angle;
  bool     crash_detected;
  uint16_t distance_cm;
  uint8_t  distance_zone;
  bool     obstacle_near;
  uint8_t  speed;
  int8_t   direction;
  bool     braking;
  bool     reversing;
  bool     platoon_active;
  bool     emergency;
  uint8_t  active_scenario;
};


struct Car2Data {
  uint8_t  vehicle_id;      // always 2
  uint8_t  msg_type;        // 1=normal 2=warn 3=emergency
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

Car1Data  car1Data;         // received FROM Car1
Car2Data  txData;           // sent TO Car1

bool          espnow_ok    = false;
bool          mpu_ok       = false;
bool          oled_ok      = false;
unsigned long car1LastSeen = 0;
bool          car1Linked   = false;
unsigned long lastSendTime = 0;

bool     ir_obstacle  = false;
float    car2_tilt    = 0.0f;
int16_t  car2_ax      = 0;
int16_t  car2_ay      = 0;
int16_t  car2_az      = 0;
bool     car2_crash   = false;
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


void stopMotors() {
  digitalWrite(IN1_PIN, LOW);
  digitalWrite(IN2_PIN, LOW);
  digitalWrite(IN3_PIN, LOW);
  digitalWrite(IN4_PIN, LOW);
  analogWrite(ENA_PIN, 0);
  analogWrite(ENB_PIN, 0);
  txData.speed     = SPEED_STOP;
  txData.direction = 0;
  txData.braking   = true;
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
  display.print("V2V CAR2");
  display.setCursor(60, 0);
  display.printf(" SC:%d %s",
    txData.active_scenario,
    car1Linked ? "[C1:OK]" : "[C1:--]");
  display.drawLine(0, 9, 128, 9, SSD1306_WHITE);

  // Car1 data
  display.setCursor(0, 12);
  if (car1Linked) {
    display.printf("C1 D:%3dcm Z:%d S:%d",
      car1Data.distance_cm,
      car1Data.distance_zone,
      car1Data.speed);
  } else {
    display.print("C1: NO SIGNAL    ");
  }

  // Car1 status
  display.setCursor(0, 22);
  if (!car1Linked)
    display.print("C1: WAITING...    ");
  else if (car1Data.crash_detected)
    display.print("C1: !! CRASH !!   ");
  else if (car1Data.emergency)
    display.print("C1: EMERGENCY!    ");
  else if (car1Data.braking)
    display.print("C1: BRAKING!      ");
  else if (car1Data.platoon_active)
    display.print("C1: PLATOON ACTIVE");
  else
    display.print("C1: RUNNING       ");

  // Car2 IR + Tilt
  display.setCursor(0, 32);
  display.printf("IR:%s TILT:%.1f",
    ir_obstacle ? "OBS" : "CLR",
    car2_tilt);

  // Car2 speed
  display.setCursor(0, 42);
  display.printf("C2 SPD:%3d DIR:%s",
    txData.speed,
    txData.direction == -1 ? "L" :
    txData.direction ==  1 ? "R" : "F");

  // Footer
  display.drawLine(0, 53, 128, 53, SSD1306_WHITE);
  display.setCursor(0, 56);
  if      (car2_crash)              display.print("!! CAR2 CRASH !!    ");
  else if (car1Data.crash_detected) display.print("!! CAR1 CRASH !!    ");
  else if (txData.emergency)        display.print("!! EMERGENCY STOP !!");
  else if (ir_obstacle)             display.print("IR OBSTACLE! ALERT  ");
  else if (!car1Linked)             display.print(">> WAITING FOR CAR1 ");
  else if (txData.platoon_active)   display.print(">> FOLLOWING CAR1   ");
  else                              display.print(">> STANDBY          ");

  display.display();
}


void onSent(uint8_t *mac, uint8_t status) {
  espnow_ok = (status == 0);
}

void onReceive(uint8_t *mac,
               uint8_t *inData, uint8_t len) {
  if (len == sizeof(Car1Data)) {
    memcpy(&car1Data, inData, sizeof(Car1Data));
    car1LastSeen = millis();
    car1Linked   = true;
  }
}


void sendToCar1() {
  if (millis() - lastSendTime < SEND_INTERVAL) return;
  lastSendTime = millis();

  txData.vehicle_id  = 2;
  txData.timestamp   = millis();
  txData.ax          = car2_ax;
  txData.ay          = car2_ay;
  txData.az          = car2_az;
  txData.tilt_angle  = car2_tilt;
  txData.crash_detected = car2_crash;
  txData.ir_obstacle = ir_obstacle;

  esp_now_send(car1MAC,
    (uint8_t*)&txData, sizeof(Car2Data));
}


void handleScenarios() {

  // Reset flags
  txData.emergency      = false;
  txData.platoon_active = false;
  txData.braking        = false;
  txData.msg_type       = 1;

  // Check Car1 link timeout
  car1Linked = ((millis() - car1LastSeen) <
                (unsigned long)CAR1_TIMEOUT
                && car1LastSeen > 0);

  
  if (car2_crash) {
    stopMotors();
    txData.emergency       = true;
    txData.msg_type        = 3;
    txData.active_scenario = 3;
    beepLong();
    showAlert("CAR2 CRASH!");
    Serial.println("SCENARIO:3 CAR2_CRASH");
    return;
  }

  // Car1 crash received → stop immediately
  if (car1Linked && car1Data.crash_detected) {
    stopMotors();
    txData.emergency       = true;
    txData.msg_type        = 3;
    txData.active_scenario = 3;
    beepLong();
    showAlert("CAR1 CRASH!");
    Serial.println("SCENARIO:3 CAR1_CRASH");
    return;
  }

 
  if (car1Linked && car1Data.emergency) {
    stopMotors();
    txData.msg_type        = 3;
    txData.active_scenario = 1;
    beepLong();
    showAlert("C1 COLLIDE!");
    Serial.println("SCENARIO:1 CAR1_EMERGENCY");
    return;
  }

  if (car1Linked && car1Data.braking) {
    stopMotors();
    txData.msg_type        = 2;
    txData.active_scenario = 1;
    beepMedium();
    showAlert("C1 BRAKE!");
    Serial.println("SCENARIO:1 CAR1_BRAKING");
    return;
  }

  
  if (ir_obstacle) {
    txData.msg_type        = 2;
    txData.active_scenario = 4;
    // Steer right when obstacle detected
    turnRight(SPEED_MEDIUM);
    beepShort();
    showAlert("IR OBS!");
    Serial.println("SCENARIO:4 IR_OBSTACLE");
    return;
  }

  // No Car1 signal → wait
  if (!car1Linked) {
    stopMotors();
    txData.active_scenario = 0;
    Serial.println("WAITING FOR CAR1...");
    return;
  }

  
  if (car1Data.distance_zone == 2) {
    moveForward(SPEED_SLOW);
    txData.msg_type        = 2;
    txData.active_scenario = 2;
    Serial.println("SCENARIO:2 ZONE2->SLOW");
    return;
  }

  if (car1Data.distance_zone == 1) {
    moveForward(SPEED_MEDIUM);
    txData.msg_type        = 2;
    txData.active_scenario = 2;
    Serial.println("SCENARIO:2 ZONE1->MEDIUM");
    return;
  }

  // Mirror Car1 direction
  if (car1Data.direction == -1) {
    turnLeft(car1Data.speed);
    txData.active_scenario = 2;
    Serial.println("SCENARIO:2 MIRROR_LEFT");
    return;
  }
  if (car1Data.direction == 1) {
    turnRight(car1Data.speed);
    txData.active_scenario = 2;
    Serial.println("SCENARIO:2 MIRROR_RIGHT");
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
    "/*%d,%d,%d,%d,%d,%.1f,%d,%d,%d,%d*/\n",
    (int)car1Linked,
    (int)car1Data.distance_cm,
    (int)car1Data.distance_zone,
    (int)car1Data.speed,
    (int)ir_obstacle,
    car2_tilt,
    (int)txData.speed,
    (int)car2_crash,
    (int)txData.emergency,
    (int)txData.active_scenario
  );
}


void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println();
  Serial.println("=== CAR2 FOLLOWER BIDIR BOOTING ===");

  // Motor pins
  pinMode(IN1_PIN, OUTPUT);
  pinMode(IN2_PIN, OUTPUT);
  pinMode(IN3_PIN, OUTPUT);
  pinMode(IN4_PIN, OUTPUT);
  pinMode(ENA_PIN, OUTPUT);
  pinMode(ENB_PIN, OUTPUT);
  stopMotors();

  // IR sensor
  pinMode(IR_PIN, INPUT_PULLUP);

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
    display.println("CAR2 BIDIR BOOT...");
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
  wifi_set_channel(1);

  if (esp_now_init() == 0) {
    // Car2 = COMBO (send AND receive)
    esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
    // Callbacks
    esp_now_register_send_cb(onSent);
    esp_now_register_recv_cb(onReceive);
    // Register Car1 as peer to send data back
    esp_now_add_peer(car1MAC,
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
  memset(&car1Data, 0, sizeof(Car1Data));
  memset(&txData,   0, sizeof(Car2Data));
  txData.vehicle_id  = 2;
  txData.msg_type    = 1;
  car1Data.distance_cm = 999;

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

  Serial.println("=== CAR2 BIDIR READY ===");
  delay(500);

  // Free TX for buzzer
  Serial.flush();
  Serial.end();
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  // Startup beep — double
  digitalWrite(BUZZER_PIN, HIGH); delay(150);
  digitalWrite(BUZZER_PIN, LOW);  delay(100);
  digitalWrite(BUZZER_PIN, HIGH); delay(150);
  digitalWrite(BUZZER_PIN, LOW);

  // Restart Serial for dashboard output
  Serial.begin(115200);
  delay(100);
}


void loop() {

  // 1. Read MPU6050
  if (mpu_ok) readMPU();

  // 2. Read IR sensor
  ir_obstacle = (digitalRead(IR_PIN) == LOW);

  // 3. Buzzer update
  buzzUpdate();

  // 4. Run 5 scenarios
  handleScenarios();

  // 5. Send Car2 data to Car1
  sendToCar1();

  // 6. Update OLED
  updateOLED();

  // 7. Serial output
  serialOutput();

  delay(LOOP_DELAY);
}
