# V2V ESP8266 Convoy — Hardware Reference

**Project:** V2V Multi-Sensor Fusion Communication System  
**Author:** Deepak M | 2025

---

## Table of Contents

1. [Wiring Guide](#wiring-guide)
2. [Hardware Components & Pinout](#hardware-components--pinout)
3. [Troubleshooting](#troubleshooting)

---

# Wiring Guide

## Power Setup (Both Cars)

Use Power Board V2 for all power distribution.

| Rail | Voltage | Powers |
|------|---------|--------|
| Rail 1 | 3.3V | MPU6050, OLED, IR Sensor, Buzzer |
| Rail 2 | 5V | L298N logic VCC |
| Rail 3 | 12V | L298N motor VIN |
| USB | 5V in | ESP8266 only (programming) |
| GND | 0V | All components — common rail |

> ⚠️ Never connect USB and 12V battery at the same time.  
> ⚠️ All GNDs must share one common ground rail.

---

## HC-SR04 Voltage Divider (Car 1 Only)

ECHO pin outputs 5V but ESP8266 GPIO max is 3.3V. Use two 1kΩ resistors:

```
HC-SR04 ECHO ──[1kΩ]──┬──[1kΩ]──  GND
                       └──  ESP8266 D5
```

---

## Car 1 — Leader Wiring

### ESP8266 to Components

| ESP8266 Pin | Connected To | Purpose |
|-------------|-------------|---------|
| D1 (GPIO5) | MPU6050 SCL + OLED SCL | I2C clock |
| D2 (GPIO4) | MPU6050 SDA + OLED SDA | I2C data |
| D3 (GPIO0) | L298N IN1 | Left motor forward |
| D4 (GPIO2) | L298N IN2 | Left motor backward |
| D5 (GPIO14) | HC-SR04 ECHO (via divider) | Distance echo input |
| D6 (GPIO12) | HC-SR04 TRIG | Distance trigger |
| D7 (GPIO13) | L298N IN3 | Right motor forward |
| D8 (GPIO15) | L298N IN4 | Right motor backward |
| D0 (GPIO16) | L298N ENA | Left motor PWM speed |
| RX (GPIO3) | L298N ENB | Right motor PWM speed |
| TX (GPIO1) | FREE | Serial Monitor output |

> ⚠️ Disconnect **RX** before uploading code. Reconnect after.

### MPU6050 Connections

| MPU6050 | Connect To |
|---------|------------|
| VCC | 3.3V |
| GND | GND rail |
| SCL | D1 |
| SDA | D2 |
| ADO | GND (sets address 0x68) |
| XDA, XCL, INT | Not connected |

### OLED Connections

| OLED | Connect To |
|------|------------|
| VCC | 3.3V |
| GND | GND rail |
| SCL | D1 (shared with MPU6050) |
| SDA | D2 (shared with MPU6050) |

### HC-SR04 Connections

| HC-SR04 | Connect To |
|---------|------------|
| VCC | 3.3V |
| GND | GND rail |
| TRIG | D6 |
| ECHO | D5 via 1kΩ+1kΩ divider |

### L298N Connections (Car 1 — 4WD)

| L298N | Connect To |
|-------|------------|
| VIN | 12V |
| VCC | 5V |
| GND | GND rail |
| IN1 | D3 |
| IN2 | D4 |
| IN3 | D7 |
| IN4 | D8 |
| ENA | D0 |
| ENB | RX |
| OUT1, OUT2 | Left motors |
| OUT3, OUT4 | Right motors |

---

## Car 2 — Follower Wiring

### ESP8266 to Components

| ESP8266 Pin | Connected To | Purpose |
|-------------|-------------|---------|
| D1 (GPIO5) | MPU6050 SCL + OLED SCL | I2C clock |
| D2 (GPIO4) | MPU6050 SDA + OLED SDA | I2C data |
| D3 (GPIO0) | L298N IN1 | Left motor forward |
| D4 (GPIO2) | L298N IN2 | Left motor backward |
| D5 (GPIO14) | L298N IN3 | Right motor forward |
| D6 (GPIO12) | L298N IN4 | Right motor backward |
| D7 (GPIO13) | L298N ENA | Left motor PWM speed |
| D8 (GPIO15) | L298N ENB | Right motor PWM speed |
| D0 (GPIO16) | IR Sensor signal | Obstacle detection |
| TX (GPIO1) | Buzzer | Audio alerts |
| RX (GPIO3) | FREE | Not connected |

> ⚠️ Disconnect **TX** (buzzer wire) before uploading. Reconnect after.  
> ❌ Do NOT connect IR to A0 — max input is 1V, IR outputs 3.3V.

### MPU6050 Connections

Same as Car 1 — VCC→3.3V, GND→rail, SCL→D1, SDA→D2, ADO→GND.

### OLED Connections

Same as Car 1 — VCC→3.3V, GND→rail, SCL→D1, SDA→D2.

### IR Sensor Connections

| IR Sensor | Connect To |
|-----------|------------|
| VCC | 3.3V |
| GND | GND rail |
| Signal | D0 |

### Buzzer Connections

| Buzzer | Connect To |
|--------|------------|
| VCC | 3.3V |
| GND | GND rail |
| Signal | TX (GPIO1) |

### L298N Connections (Car 2 — 2WD)

| L298N | Connect To |
|-------|------------|
| VIN | 12V |
| VCC | 5V |
| GND | GND rail |
| IN1 | D3 |
| IN2 | D4 |
| IN3 | D5 |
| IN4 | D6 |
| ENA | D7 |
| ENB | D8 |
| OUT1, OUT2 | Left motor |
| OUT3, OUT4 | Right motor |

---

## Upload Procedure

**Car 1:**
1. Disconnect RX wire from ESP8266
2. Upload code in PlatformIO
3. Reconnect RX wire

**Car 2:**
1. Disconnect TX wire (Buzzer) from ESP8266
2. Upload code in PlatformIO
3. Reconnect TX wire

---

---

# Hardware Components & Pinout

## Car 1 — Leader

### Bill of Materials

| # | Component | Model | Qty |
|---|-----------|-------|-----|
| 1 | Microcontroller | ESP8266 NodeMCU v2 | 1 |
| 2 | IMU | MPU-6050 | 1 |
| 3 | Ultrasonic Sensor | HC-SR04 | 1 |
| 4 | Display | SSD1306 OLED 128x64 | 1 |
| 5 | Motor Driver | L298N Dual H-Bridge | 1 |
| 6 | DC Gear Motor | 6V 200RPM | 4 |
| 7 | Power Board | V2 (3.3V/5V/12V) | 1 |
| 8 | Battery | 12V LiPo or Lead-Acid | 1 |
| 9 | Resistor | 1kΩ carbon film | 2 |
| 10 | Chassis | 4WD acrylic platform | 1 |

### ESP8266 NodeMCU v2 — Specs

| Spec | Value |
|------|-------|
| CPU | Tensilica L106 32-bit @ 80MHz |
| Flash | 4MB |
| GPIO | 11 digital + 1 analog (A0) |
| I2C | Software I2C on any GPIO |
| WiFi | 802.11 b/g/n 2.4GHz (ESP-NOW) |
| Voltage | 3.3V logic, 5V USB input |
| ESP-NOW role | ESP_NOW_ROLE_COMBO |
| Upload note | Disconnect RX before upload |

### MPU6050 — Specs

| Spec | Value |
|------|-------|
| Axes | 3-axis accel + 3-axis gyro |
| I2C address | 0x68 (ADO pin = GND) |
| Supply | 3.3V |
| Accel range | ±2g default |
| Tilt formula | atan2(ax, sqrt(ay²+az²)) × 180/π |
| Crash threshold | \|ax\| or \|ay\| > 12000 raw |

### HC-SR04 — Specs

| Spec | Value |
|------|-------|
| Range | 2cm – 400cm |
| Accuracy | ±3mm |
| Supply | 3.3V |
| TRIG pin | D6 — 10µs HIGH pulse |
| ECHO pin | D5 via 1kΩ+1kΩ divider |
| Formula | distance = pulse_us × 0.034 / 2 |

### Distance Zones

| Zone | Distance | Speed |
|------|----------|-------|
| 0 — SAFE | > 50cm | 220 (FAST) |
| 1 — WARNING | 20–50cm | 160 (MEDIUM) |
| 2 — DANGER | 11–20cm | 80 (SLOW) |
| 3 — CRITICAL | ≤ 11cm | 0 (STOP) |

### Full Pinout — Car 1

| Pin | GPIO | Type | Connected To |
|-----|------|------|-------------|
| D1 | GPIO5 | I2C | MPU6050 SCL + OLED SCL |
| D2 | GPIO4 | I2C | MPU6050 SDA + OLED SDA |
| D3 | GPIO0 | Motor | L298N IN1 |
| D4 | GPIO2 | Motor | L298N IN2 |
| D5 | GPIO14 | Sensor | HC-SR04 ECHO (divider) |
| D6 | GPIO12 | Sensor | HC-SR04 TRIG |
| D7 | GPIO13 | Motor | L298N IN3 |
| D8 | GPIO15 | Motor | L298N IN4 |
| D0 | GPIO16 | Motor | L298N ENA (PWM) |
| RX | GPIO3 | Motor | L298N ENB (PWM) ⚠️ |
| TX | GPIO1 | Serial | FREE — Serial Monitor |
| A0 | ADC0 | — | NOT USED |
| 3.3V | — | Power | MPU6050 + OLED VCC |
| 5V | — | Power | L298N logic VCC |
| 12V | — | Power | L298N motor VIN |
| GND | — | Ground | All component GNDs |

---

## Car 2 — Follower

### Bill of Materials

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
| 9 | Battery | 12V LiPo or Lead-Acid | 1 |
| 10 | Chassis | 2WD acrylic platform | 1 |

### IR Sensor — Specs

| Spec | Value |
|------|-------|
| Model | TCRT5000 |
| Supply | 3.3V |
| Signal pin | D0 (INPUT_PULLUP) |
| Output | LOW = obstacle, HIGH = clear |
| Range | 2–30cm (potentiometer adjustable) |
| Mount | Center front of chassis |
| Triggers | Scenario 4 — Cooperative Avoidance |

### Buzzer — Specs

| Spec | Value |
|------|-------|
| Type | Active buzzer |
| Supply | 3.3V |
| Signal pin | TX (GPIO1) |
| Short beep 150ms | Scenario 4 — IR obstacle |
| Medium beep 400ms | Scenario 1 — Car 1 braking |
| Long beep 900ms | Scenario 3 — Crash detected |
| Non-blocking | Uses millis() timer |

### Full Pinout — Car 2

| Pin | GPIO | Type | Connected To |
|-----|------|------|-------------|
| D1 | GPIO5 | I2C | MPU6050 SCL + OLED SCL |
| D2 | GPIO4 | I2C | MPU6050 SDA + OLED SDA |
| D3 | GPIO0 | Motor | L298N IN1 |
| D4 | GPIO2 | Motor | L298N IN2 |
| D5 | GPIO14 | Motor | L298N IN3 |
| D6 | GPIO12 | Motor | L298N IN4 |
| D7 | GPIO13 | Motor | L298N ENA (PWM) |
| D8 | GPIO15 | Motor | L298N ENB (PWM) |
| D0 | GPIO16 | Sensor | IR sensor signal |
| TX | GPIO1 | Buzzer | Buzzer signal ⚠️ |
| RX | GPIO3 | — | FREE — not connected |
| A0 | ADC0 | — | NOT USED ❌ max 1V |
| 3.3V | — | Power | MPU6050 + OLED + IR + Buzzer VCC |
| 5V | — | Power | L298N logic VCC |
| 12V | — | Power | L298N motor VIN |
| GND | — | Ground | All component GNDs |

---

## Car 1 vs Car 2 — Quick Comparison

| Feature | Car 1 Leader | Car 2 Follower |
|---------|-------------|----------------|
| Drive | 4WD — 4 motors | 2WD — 2 motors |
| Distance sensor | HC-SR04 on D5/D6 | None |
| IR sensor | None | TCRT5000 on D0 |
| Buzzer | None | Active buzzer on TX |
| Motor speed pins | ENA=D0, ENB=RX | ENA=D7, ENB=D8 |
| Motor dir pins | IN3/4 on D7/D8 | IN3/4 on D5/D6 |
| TX pin use | Serial Monitor | Buzzer output |
| RX pin use | L298N ENB (motor) | FREE |
| Upload warning | Disconnect RX | Disconnect TX |

---

## I2C Addresses

| Device | Address | Notes |
|--------|---------|-------|
| MPU6050 | 0x68 | ADO pin tied to GND |
| SSD1306 OLED | 0x3C | Default for 128x64 |

## Speed Constants

| Constant | PWM | Used When |
|----------|-----|-----------|
| SPEED_STOP | 0 | Emergency, crash, blocked |
| SPEED_SLOW | 80 | Zone 2 DANGER |
| SPEED_MEDIUM | 160 | Zone 1 WARNING |
| SPEED_FAST | 220 | Zone 0 SAFE, platoon |

## MAC Addresses

| Vehicle | MAC | Configured In |
|---------|-----|---------------|
| Car 1 | C4:5B:BE:71:31:47 | car2_bidirectional/src/main.cpp |
| Car 2 | C8:C9:A3:66:97:D1 | car1_bidirectional/src/main.cpp |

---

---

# Troubleshooting

## Upload Issues

| Problem | Cause | Fix |
|---------|-------|-----|
| Upload fails on Car 1 | RX wire connected | Disconnect RX, upload, reconnect |
| Upload fails on Car 2 | TX wire (buzzer) connected | Disconnect TX, upload, reconnect |
| `COM3 PermissionError` | Port in use by Serial Monitor | Close Serial Monitor first |
| Port not found | Wrong COM port | Check Device Manager, update platformio.ini |

---

## ESP-NOW Issues

| Problem | Cause | Fix |
|---------|-------|-----|
| LINK shows WAIT forever | Wrong MAC address | Run test_espnow to get correct MACs |
| LINK shows WAIT forever | Different WiFi channels | Both must use channel 1 |
| Packets lost > 1% | Distance too far | Move cars within 5m for testing |
| Car 2 not reacting | car1Data not updating | Check onReceive callback is registered |

---

## Sensor Issues

| Problem | Cause | Fix |
|---------|-------|-----|
| MPU6050 tilt always 0.0 | Wrong formula | Use `atan2(ax, sqrt(ay²+az²))` |
| MPU6050 not found | Wrong I2C address | Confirm ADO pin is at GND (address 0x68) |
| OLED blank | Wrong I2C address | Run I2C scanner sketch, verify 0x3C |
| HC-SR04 always 999cm | Divider not connected | Check 1kΩ+1kΩ divider on ECHO pin |
| HC-SR04 noisy readings | No averaging | Take 3 readings, use median value |
| IR false triggers | Wrong pin | Only use D0 — never RX (GPIO3) |
| IR always LOW | Wired wrong | Check VCC=3.3V, GND=rail, Signal=D0 |
| A0 IR not working | A0 voltage limit | A0 max is 1V — IR outputs 3.3V, use D0 |

---

## Motor Issues

| Problem | Cause | Fix |
|---------|-------|-----|
| Motors not spinning | L298N not powered | Check 12V on VIN, 5V on VCC |
| One motor wrong direction | OUT wires swapped | Swap OUT1/OUT2 or OUT3/OUT4 |
| Motors jitter at low speed | PWM value too low | Use minimum SPEED_SLOW=80 |
| Car drives in circle | Motor speeds unequal | Calibrate ENA/ENB PWM values |
| No speed control | ENA/ENB jumpers on | Remove jumpers from L298N ENA/ENB |

---

## Buzzer Issues

| Problem | Cause | Fix |
|---------|-------|-----|
| Buzzer beeps at boot | TX active during Serial.begin | Call `Serial.end()` before using TX as output |
| Buzzer always on | pinMode not set | Add `pinMode(1, OUTPUT)` after Serial.end() |
| Buzzer beeps during upload | TX is programming line | Disconnect buzzer wire before upload |
| No sound | Wrong pin | Confirm buzzer signal on TX (GPIO1) |

---

## Python Dashboard Issues

| Problem | Cause | Fix |
|---------|-------|-----|
| `PermissionError` on COM3 | Serial Monitor open | Close PlatformIO Serial Monitor |
| No data on dashboard | Wrong COM port | Check port in dashboard script |
| Graph not updating | Serial format wrong | Verify `/*...,*/` format in firmware |
| Dashboard crash on start | Missing libraries | Run `pip install pyserial matplotlib numpy` |

---

## Power Issues

| Problem | Cause | Fix |
|---------|-------|-----|
| ESP8266 keeps rebooting | Insufficient power | Use USB + Power Board, not USB alone |
| Motors stop randomly | Voltage sag | Ensure 12V battery is charged |
| Sensors give wrong data | Noisy power | Add 100µF capacitor across Power Board output |
| Components get hot | Wrong voltage | Double-check 3.3V rail, not 5V to sensors |

---

## Common Pin Mistakes

| Mistake | Result | Correct |
|---------|--------|---------|
| IR sensor on RX (GPIO3) | False triggers at boot | Use D0 (GPIO16) |
| IR sensor on A0 | ADC damage — max 1V | Use D0 (GPIO16) |
| HC-SR04 ECHO direct to D5 | GPIO damage — 5V input | Use 1kΩ+1kΩ divider |
| Buzzer on RX instead of TX | No buzzer, boot issues | TX (GPIO1) only |
| USB + 12V battery together | ESP8266 damage | Use one power source at a time |
| Missing common GND | Sensors read garbage | Connect all GNDs to one rail |
