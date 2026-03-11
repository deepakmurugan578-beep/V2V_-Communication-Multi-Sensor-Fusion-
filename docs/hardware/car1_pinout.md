# Car 1 — Leader Pinout

**Vehicle:** Car 1 — Leader (4WD)  
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
| D5 | GPIO14 | Sensor | HC-SR04 ECHO | Via 1kΩ+1kΩ divider |
| D6 | GPIO12 | Sensor | HC-SR04 TRIG | Direct connection |
| D7 | GPIO13 | Motor | L298N IN3 | Right motor forward |
| D8 | GPIO15 | Motor | L298N IN4 | Right motor backward |
| D0 | GPIO16 | Motor | L298N ENA | Left motor PWM speed |
| RX | GPIO3 | Motor | L298N ENB | Right motor PWM —  disconnect before upload |
| TX | GPIO1 | Serial | FREE | Serial Monitor output |
| A0 | ADC0 | — | NOT USED | — |
| 3.3V | — | Power | MPU6050 VCC + OLED VCC | From Power Board |
| 5V | — | Power | L298N logic VCC | From Power Board |
| 12V | — | Power | L298N motor VIN | From Power Board |
| GND | — | Ground | All component GNDs | Common rail |

---

## Bill of Materials

| # | Component | Model | Qty |
|---|-----------|-------|-----|
| 1 | Microcontroller | ESP8266 NodeMCU v2 | 1 |
| 2 | IMU | MPU-6050 | 1 |
| 3 | Ultrasonic Sensor | HC-SR04 | 1 |
| 4 | Display | SSD1306 OLED 128x64 | 1 |
| 5 | Motor Driver | L298N Dual H-Bridge | 1 |
| 6 | DC Gear Motor | 6V 200RPM | 4 |
| 7 | Power Board | V2 (3.3V/5V/12V) | 1 |
| 8 | Battery | 12V | 1 |
| 9 | Resistor | 1kΩ | 2 |
| 10 | Chassis | 4WD acrylic platform | 1 |

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
| MAC address | C4:5B:BE:71:31:47 |
| Upload note | Disconnect RX before upload |

### MPU-6050

| Spec | Value |
|------|-------|
| I2C address | 0x68 (ADO = GND) |
| Supply | 3.3V |
| Axes | 3-axis accel + 3-axis gyro |
| Tilt formula | atan2(ax, sqrt(ay²+az²)) × 180/π |
| Crash threshold | \|ax\| or \|ay\| > 12000 raw |

### HC-SR04

| Spec | Value |
|------|-------|
| Range | 2cm – 400cm |
| Accuracy | ±3mm |
| Supply | 3.3V |
| TRIG | D6 — 10µs HIGH pulse |
| ECHO | D5 via 1kΩ+1kΩ divider |
| Formula | distance_cm = pulse_µs × 0.034 / 2 |

### Distance Zones

| Zone | Distance | Car 1 Speed | Car 2 Speed |
|------|----------|------------|------------|
| 0 — SAFE | > 50cm | 220 | 220 |
| 1 — WARNING | 20–50cm | 160 | 160 |
| 2 — DANGER | 11–20cm | 80 | 80 |
| 3 — CRITICAL | ≤ 11cm | 0 (STOP) | 0 (STOP) |

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

## Serial Output Format

```
/*DIST,ZONE,TILT,SPD,DIR,MSG,CRASH,BRAKE,LINK,SCENARIO*/
/*45,1,4.6,160,0,2,0,0,1,2*/
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
