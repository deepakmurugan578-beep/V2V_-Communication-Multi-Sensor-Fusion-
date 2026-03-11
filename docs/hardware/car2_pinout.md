# Car 2 — Follower Pinout

**Vehicle:** Car 2 — Follower (2WD)  
**MCU:** ESP8266 NodeMCU v2  
**Author:** Deepak M | 2025

---

## Full Pinout

| Pin | GPIO | Type | Connected To | Notes |
|-----|------|------|-------------|-------|
| D1 | GPIO5 | I2C | MPU6050 SCL + OLED SCL | Shared I2C clock |
| D2 | GPIO4 | I2C | MPU6050 SDA + OLED SDA | Shared I2C data |
| D3 | GPIO0 | Motor | L298N IN1 | Left motor forward |
| D4 | GPIO2 | Motor | L298N IN2 | Left motor backward |
| D5 | GPIO14 | Motor | L298N IN3 | Right motor forward |
| D6 | GPIO12 | Motor | L298N IN4 | Right motor backward |
| D7 | GPIO13 | Motor | L298N ENA | Left motor PWM speed |
| D8 | GPIO15 | Motor | L298N ENB | Right motor PWM speed |
| D0 | GPIO16 | Sensor | IR Sensor signal | INPUT_PULLUP — LOW = obstacle |
| TX | GPIO1 | Buzzer | Buzzer signal |  disconnect before upload |
| RX | GPIO3 | — | FREE | Not connected |
| A0 | ADC0 | — | NOT USED |  max 1V — do not use for IR |
| 3.3V | — | Power | MPU6050 + OLED + IR + Buzzer VCC | From Power Board |
| 5V | — | Power | L298N logic VCC | From Power Board |
| 12V | — | Power | L298N motor VIN | From Power Board |
| GND | — | Ground | All component GNDs | Common rail |

---

## Bill of Materials

| # | Component | Model | Qty |
|---|-----------|-------|-----|
| 1 | Microcontroller | ESP8266 NodeMCU v2 | 1 |
| 2 | IMU | MPU-6050 | 1 |
| 3 | IR Obstacle Sensor | TCRT5000 | 1 |
| 4 | Display | SSD1306 OLED 128x64 | 1 |
| 5 | Buzzer | Active Buzzer | 1 |
| 6 | Motor Driver | L298N Dual H-Bridge | 1 |
| 7 | DC Gear Motor | 6V 200RPM | 2 |
| 8 | Power Board | V2 (3.3V/5V/12V) | 1 |
| 9 | Battery | 12V | 1 |
| 10 | Chassis | 2WD acrylic platform | 1 |

---

## Component Specs

### ESP8266 NodeMCU v2

| Spec | Value |
|------|-------|
| CPU | Tensilica L106 32-bit @ 80MHz |
| Flash | 4MB |
| GPIO | 11 digital + 1 analog |
| Voltage | 3.3V logic, 5V USB in |
| ESP-NOW role | ESP_NOW_ROLE_COMBO |
| MAC address | C8:C9:A3:66:97:D1 |
| Upload note | Disconnect TX before upload |

### MPU-6050

| Spec | Value |
|------|-------|
| I2C address | 0x68 (ADO = GND) |
| Supply | 3.3V |
| Axes | 3-axis accel + 3-axis gyro |
| Tilt formula | atan2(ax, sqrt(ay²+az²)) × 180/π |
| Crash threshold | \|ax\| or \|ay\| > 12000 raw |
| On crash | Stops + sends crash_detected=true to Car 1 |

### IR Obstacle Sensor (TCRT5000)

| Spec | Value |
|------|-------|
| Supply | 3.3V |
| Signal pin | D0 (INPUT_PULLUP) |
| Output | LOW = obstacle, HIGH = clear |
| Range | 2–30cm (adjustable) |
| Mount | Center front of chassis |
| Triggers | Scenario 4 — Cooperative Avoidance |
| Safe pins | D0 only — not RX, not A0 |

### Active Buzzer

| Spec | Value |
|------|-------|
| Supply | 3.3V |
| Signal pin | TX (GPIO1) |
| Short beep 150ms | Scenario 4 — IR obstacle |
| Medium beep 400ms | Scenario 1 — Car 1 braking |
| Long beep 900ms | Scenario 3 — Crash detected |
| Non-blocking | Uses millis() timer |
| Firmware | Serial.end() → pinMode(1, OUTPUT) → Serial.begin() |

### SSD1306 OLED

| Spec | Value |
|------|-------|
| Resolution | 128 × 64 px |
| I2C address | 0x3C |
| Supply | 3.3V |
| Library | Adafruit SSD1306 v2.5.7 |

### L298N Motor Driver

| Spec | Value |
|------|-------|
| Motor supply | 12V |
| Logic supply | 5V |
| Max current | 2A per channel |
| PWM range | 0–255 (analogWrite) |

---

## Speed Constants

| Constant | PWM | Used When |
|----------|-----|-----------|
| SPEED_STOP | 0 | Emergency, crash, blocked |
| SPEED_SLOW | 80 | Zone 2 DANGER |
| SPEED_MEDIUM | 160 | Zone 1 WARNING, IR avoidance |
| SPEED_FAST | 220 | Zone 0 SAFE, platoon |

---

## Serial Output Format

```
/*LINK,C1DIST,C1ZONE,C1SPD,IR,TILT,C2SPD,CRASH,EMRG,SCENARIO*/
/*1,45,1,160,0,4.6,160,0,0,5*/
```

---

## platformio.ini

```ini
[env:nodemcuv2]
platform = espressif8266
board = nodemcuv2
framework = arduino
monitor_speed = 115200
upload_port = COM3
monitor_port = COM3
lib_deps =
    adafruit/Adafruit SSD1306@^2.5.7
    adafruit/Adafruit GFX Library@^1.11.9
```
