
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
#define ENB       3    

#define SPEED_STOP    0
#define SPEED_SLOW    80
#define SPEED_MEDIUM  160
#define SPEED_FAST    220

#define DIST_CRITICAL  11   
#define DIST_DANGER    20 
#define DIST_WARNING   50   

#define CRASH_THRESHOLD 12000  

#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT  64


Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT,
                          &Wire, -1);


struct V2VData {
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
};

V2VData carData;


uint8_t receiverMAC[] = {0xC8, 0xC9, 0xA3, 0x66, 0x97, 0xD1};

bool espnow_ok = false;
bool mpu_ok    = false;
bool oled_ok   = false;


#define MPU_ADDR 0x68

void mpuWrite(byte reg, byte val) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(reg);
  Wire.write(val);
  Wire.endTransmission(true);
}

int16_t mpuReadWord(byte reg) {
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
  mpuWrite(0x6B, 0x00);
  delay(100);
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x75);
  Wire.endTransmission(false);
  Wire.requestFrom((uint8_t)MPU_ADDR,
                   (uint8_t)1,
                   (uint8_t)true);
  if (!Wire.available()) return false;
  byte id = Wire.read();
  return (id == 0x68 || id == 0x70 || id == 0x72);
}

void readMPU() {
  carData.ax = mpuReadWord(0x3B);
  carData.ay = mpuReadWord(0x3D);
  carData.az = mpuReadWord(0x3F);
  carData.gx = mpuReadWord(0x43);
  carData.gy = mpuReadWord(0x45);
  carData.gz = mpuReadWord(0x47);
  carData.tilt_angle = atan2(
    (float)carData.ay,
    (float)carData.az) * 180.0 / PI;
}


float readDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long dur = pulseIn(ECHO_PIN, HIGH, 30000);
  if (dur == 0) return -1;
  return (float)(dur * 0.034 / 2.0);
}

uint8_t getDistanceZone(float cm) {
  if (cm <= 0)             return 0;
  if (cm <= DIST_CRITICAL) return 3;
  if (cm <= DIST_DANGER)   return 2;
  if (cm <= DIST_WARNING)  return 1;
  return 0;
}

void stopMotors() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
}

void setMotors(bool lf, bool lb,
               bool rf, bool rb,
               int  ls, int  rs) {
  digitalWrite(IN1, lf);
  digitalWrite(IN2, lb);
  digitalWrite(IN3, rf);
  digitalWrite(IN4, rb);
  analogWrite(ENA, ls);
  analogWrite(ENB, rs);
}

void moveForward(int spd) {
  setMotors(1, 0, 1, 0, spd, spd);
}

void moveBackward(int spd) {
  setMotors(0, 1, 0, 1, spd, spd);
}

void turnLeft(int spd) {
  setMotors(1, 0, 1, 0, spd / 2, spd);
}

void turnRight(int spd) {
  setMotors(1, 0, 1, 0, spd, spd / 2);
}

//
String       alertMsg  = "";
unsigned long alertTime = 0;
#define ALERT_DURATION 1500

void showAlert(String msg) {
  alertMsg  = msg;
  alertTime = millis();
}

void updateOLED() {
  if (!oled_ok) return;
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  if (alertMsg != "" &&
      millis() - alertTime < ALERT_DURATION) {
    display.setTextSize(2);
    display.setCursor(10, 20);
    display.println(alertMsg);
    display.display();
    return;
  }
  alertMsg = "";

  display.setTextSize(1);

  
  display.setCursor(0, 0);
  display.print("V2V CAR1");
  display.setCursor(72, 0);
  display.println(carData.platoon_active ?
    "[PLATOON]" : "[LEADER] ");
  display.drawLine(0, 9, 128, 9, SSD1306_WHITE);

  // Distance
  display.setCursor(0, 12);
  if (carData.distance_cm == 999)
    display.print("DIST: ---cm");
  else
    display.printf("DIST: %3dcm", carData.distance_cm);
  switch (carData.distance_zone) {
    case 0: display.println(" SAFE");     break;
    case 1: display.println(" WARN");     break;
    case 2: display.println(" DANGER");   break;
    case 3: display.println(" CRITICAL"); break;
  }

  // Tilt
  display.setCursor(0, 22);
  display.printf("TILT: %.1f deg", carData.tilt_angle);
  if (abs(carData.tilt_angle) > 30)
    display.println(" !");
  else
    display.println("   ");

  // ESP-NOW link
  display.setCursor(0, 32);
  display.print("LINK: ");
  display.println(espnow_ok ? "CONNECTED" : "WAITING..");

  // Speed + Direction
  display.setCursor(0, 42);
  display.printf("SPD:%3d DIR:", carData.speed);
  switch (carData.direction) {
    case -1: display.println("LEFT "); break;
    case  1: display.println("RGHT "); break;
    default: display.println("FWRD "); break;
  }

  // Status bar
  display.drawLine(0, 53, 128, 53, SSD1306_WHITE);
  display.setCursor(0, 56);
  if      (carData.crash_detected)
    display.println("!! CRASH DETECTED !!");
  else if (carData.emergency)
    display.println("!! EMERGENCY STOP !!");
  else if (carData.braking)
    display.println("!! BRAKING        !!");
  else if (carData.platoon_active)
    display.println(">> PLATOON RUNNING   ");
  else
    display.println(">> STANDBY           ");

  display.display();
}


void onSent(uint8_t *mac, uint8_t status) {
  espnow_ok = (status == 0);
}


void handleScenarios() {

  
  carData.emergency      = false;
  carData.crash_detected = false;
  carData.braking        = false;
  carData.reversing      = false;
  carData.msg_type       = 1;

  
  if (abs(carData.ax) > CRASH_THRESHOLD ||
      abs(carData.ay) > CRASH_THRESHOLD) {
    carData.crash_detected = true;
    carData.emergency      = true;
    carData.msg_type       = 3;
    carData.speed          = SPEED_STOP;
    carData.direction      = 0;
    stopMotors();
    showAlert("!! CRASH !!");
    Serial.println(" CRASH DETECTED!");
    return;
  }

  
  if (carData.distance_zone == 3) {
    carData.emergency = true;
    carData.braking   = true;
    carData.msg_type  = 3;
    carData.speed     = SPEED_STOP;
    carData.direction = 0;
    stopMotors();
    showAlert("COLLISION!");
    Serial.println(" COLLISION! STOP!");
    return;
  }

  
  if (carData.distance_zone == 2) {
    carData.msg_type  = 2;
    carData.speed     = SPEED_SLOW;
    carData.direction = 0;
    moveForward(SPEED_SLOW);
    Serial.println("  DANGER → SLOW");
    return;
  }

 
  if (carData.distance_zone == 1) {
    carData.msg_type  = 2;
    carData.speed     = SPEED_MEDIUM;
    carData.direction = 0;
    moveForward(SPEED_MEDIUM);
    Serial.println(" WARNING → MEDIUM");
    return;
  }

  
  carData.platoon_active = true;
  carData.msg_type       = 1;
  carData.speed          = SPEED_FAST;
  carData.direction      = 0;
  moveForward(SPEED_FAST);
}


void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n\n=== CAR1 LEADER BOOTING ===");

  
  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT); pinMode(ENB, OUTPUT);
  stopMotors();

  
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  digitalWrite(TRIG_PIN, LOW);

  
  pinMode(SDA_PIN, INPUT_PULLUP);
  pinMode(SCL_PIN, INPUT_PULLUP);
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(100000);
  delay(100);

 
  if (display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    oled_ok = true;
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("CAR1 BOOTING...");
    display.display();
    Serial.println(" OLED OK");
  } else {
    Serial.println(" OLED FAILED");
  }

  delay(500);

  if (mpuBegin()) {
    mpu_ok = true;
    Serial.println(" MPU6050 OK");
    if (oled_ok) {
      display.setCursor(0, 10);
      display.println("MPU6050 OK");
      display.display();
    }
  } else {
    Serial.println(" MPU6050 FAILED");
    if (oled_ok) {
      display.setCursor(0, 10);
      display.println("MPU6050 FAIL!");
      display.display();
    }
  }

  delay(500);

  // ── ESP-NOW ──
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  if (esp_now_init() == 0) {
    esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
    esp_now_register_send_cb(onSent);
    esp_now_add_peer(receiverMAC,
      ESP_NOW_ROLE_SLAVE, 1, NULL, 0);
    Serial.println(" ESP-NOW OK");
    if (oled_ok) {
      display.setCursor(0, 20);
      display.println("ESP-NOW OK");
      display.display();
    }
  } else {
    Serial.println(" ESP-NOW FAILED");
    if (oled_ok) {
      display.setCursor(0, 20);
      display.println("ESP-NOW FAIL!");
      display.display();
    }
  }

  // ── Init data ──
  carData.vehicle_id     = 1;
  carData.msg_type       = 1;
  carData.platoon_active = false;
  carData.emergency      = false;
  carData.speed          = 0;
  carData.direction      = 0;
  carData.distance_cm    = 999;

  delay(1500);

  if (oled_ok) {
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(20, 20);
    display.println("CAR1");
    display.setCursor(10, 42);
    display.println("READY!");
    display.display();
    delay(1000);
  }

  Serial.println("=== CAR1 READY ===\n");
}


void loop() {

  // ── 1. Read MPU6050 ──
  if (mpu_ok) {
    readMPU();
  }

  // ── 2. Read HC-SR04 ──
  float dist = readDistance();
  if (dist < 0) {
    carData.distance_cm   = 999;
    carData.distance_zone = 0;
    carData.obstacle_near = false;
  } else {
    carData.distance_cm   = (uint16_t)dist;
    carData.distance_zone = getDistanceZone(dist);
    carData.obstacle_near = (dist <= DIST_DANGER);
  }

  // ── 3. Timestamp ──
  carData.timestamp = millis();

  // ── 4. Handle Scenarios + Drive ──
  handleScenarios();

  // ── 5. Send via ESP-NOW ──
  esp_now_send(receiverMAC,
    (uint8_t*)&carData, sizeof(carData));

  // ── 6. Update OLED ──
  updateOLED();

  // ── 7. Serial Debug ──
  Serial.printf(
    "[CAR1] DIST:%3dcm ZONE:%d "
    "TILT:%.1f SPD:%d DIR:%d "
    "MSG:%d LINK:%s\n",
    carData.distance_cm,
    carData.distance_zone,
    carData.tilt_angle,
    carData.speed,
    carData.direction,
    carData.msg_type,
    espnow_ok ? "OK" : "WAIT"
  );

  delay(100);
}
