<div align="center">

<br/>

```
<<<<<<< HEAD
       ██╗   ██╗██████╗ ██╗   ██╗    ███████╗███████╗██████╗
       ██║   ██║╚════██╗██║   ██║    ██╔════╝██╔════╝██╔══██╗
       ██║   ██║ █████╔╝██║   ██║    █████╗  ███████╗██████╔╝
       ╚██╗ ██╔╝██╔═══╝ ╚██╗ ██╔╝    ██╔══╝  ╚════██║██╔═══╝
        ╚████╔╝ ███████╗ ╚████╔╝     ███████╗███████║██║
         ╚═══╝  ╚══════╝  ╚═══╝      ╚══════╝╚══════╝╚═╝
=======
                                  ██╗   ██╗██████╗ ██╗   ██╗   
                                  ██║   ██║╚════██╗██║   ██║    
                                  ██║   ██║ █████╔╝██║   ██║   
                                  ╚██╗ ██╔╝██╔═══╝ ╚██╗ ██╔╝   
                                   ╚████╔╝ ███████╗ ╚████╔╝    
                                    ╚═══╝  ╚══════╝  ╚═══╝     
>>>>>>> 07338ea0daa62cd4a166346d3483aa8157aa6346
```

# V2V Multi-Sensor Fusion Communication System

### *Autonomous Convoy Intelligence via ESP-NOW Protocol*

<br/>

[![ESP8266](https://img.shields.io/badge/MCU-ESP8266%20NodeMCU-blue?style=for-the-badge&logo=espressif&logoColor=white)](https://www.espressif.com/)
[![ESP-NOW](https://img.shields.io/badge/Protocol-ESP--NOW%20Bidirectional-orange?style=for-the-badge&logo=wifi&logoColor=white)]()
[![PlatformIO](https://img.shields.io/badge/IDE-PlatformIO-purple?style=for-the-badge&logo=platformio&logoColor=white)](https://platformio.org/)
[![Python](https://img.shields.io/badge/Dashboard-Python%203.14-yellow?style=for-the-badge&logo=python&logoColor=white)](https://python.org/)
[![License](https://img.shields.io/badge/License-MIT-green?style=for-the-badge)]()
[![Status](https://img.shields.io/badge/Status-Complete%20✓-brightgreen?style=for-the-badge)]()

<br/>

> **A real-time, peer-to-peer vehicle-to-vehicle (V2V) communication system built on ESP8266 microcontrollers using the ESP-NOW protocol — enabling sub-millisecond latency convoy coordination, crash detection, obstacle sharing, and autonomous platooning across 5 distinct safety scenarios.**

<br/>

**Deepak M** · Embedded Systems & IoT Engineer
<br/>

---

</div>

<br/>

##  Table of Contents

- [Project Overview](#-project-overview)
- [System Architecture](#-system-architecture)
- [Hardware Components](#-hardware-components)
- [Pin Maps](#-pin-maps)
- [5 Safety Scenarios](#-5-safety-scenarios)
- [ESP-NOW Protocol](#-esp-now-protocol)
- [Repository Structure](#-repository-structure)
- [Getting Started](#-getting-started)
- [Python Dashboards](#-python-dashboards)
- [Data Packets](#-data-packets)
- [Results & Performance](#-results--performance)
- [Troubleshooting](#-troubleshooting)
- [Future Work](#-future-work)
- [Author](#-author)

<br/>

---

##  Project Overview

Modern autonomous vehicles rely on **Vehicle-to-Vehicle (V2V) communication** to share real-time sensor data, enabling safer and more intelligent collective behavior than any single vehicle can achieve alone. This project implements a fully functional V2V system using low-cost ESP8266 microcontrollers demonstrating that advanced convoy intelligence is achievable without expensive hardware.

### Key Innovations

| Feature | Description |
|---------|-------------|
| **Bidirectional ESP-NOW** | Both cars send and receive simultaneously with ~1ms latency |
| **Multi-Sensor Fusion** | MPU6050 + HC-SR04 + IR sensors fused for comprehensive situational awareness |
| **5 Safety Scenarios** | Collision, following, crash, obstacle sharing, platoon — all active simultaneously |
| **Real-Time Dashboard** | Live Python dashboards with arc gauges, graphs, and alert panels for both vehicles |
| **No Infrastructure** | Peer-to-peer communication — no WiFi router, no internet required |
| **Modular Codebase** | Separate original and bidirectional implementations for clean version control |

   
##  System Architecture

```

  ┌──────────────────────────────────────────────────────────┐
  │                   CAR 1                        │
  │                                                          │
  │  [MPU6050]   [HC-SR04]   [OLED 128x64]                   │
  │      │           │            │                          │
  │      └───────────┴────────────┘                          │
  │                  │                                       │
  │          [ESP8266 NodeMCU]                               │
  │                  │                                       │
  │          [L298N + 4 Motors]                              │
  └──────────────────┬───────────────────────────────────────┘
                     │
          ┌──────────▼──────────┐
          │   ESP-NOW Protocol  │
          │   Bidirectional     │
          │   ~1ms latency      │
          │   No router needed  │
          └──────────┬──────────┘
                     │
  ┌──────────────────▼───────────────────────────────────────┐
  │                   CAR 2                       │
  │                                                          │
  │  [MPU6050]  [IR Sensor]  [OLED 128x64]  [Buzzer]        │
  │      │           │            │              │           │
  │      └───────────┴────────────┴──────────────┘           │
  │                  │                                       │
  │          [ESP8266 NodeMCU]                               │
  │                  │                                       │
  │          [L298N + 2 Motors]                              │
  └──────────────────┬───────────────────────────────────────┘
                     │
          ┌──────────▼──────────┐
          │  Python Dashboards  │
          │  Car1 + Car2 live   │
          │  via Serial / USB   │
          └─────────────────────┘
```

<br/>

---

##  Hardware Components

### Car 1 —  (4-Wheel Drive)

| Component | Model | Specification | Purpose |
|-----------|-------|---------------|---------|
| Microcontroller | ESP8266 NodeMCU v2 | 80MHz, 3.3V | Main controller + ESP-NOW |
| IMU | MPU-6050 | 6-axis, I2C 0x68 | Tilt detection + crash sensing |
| Ultrasonic | HC-SR04 | 2–400cm, ±3mm | Forward distance measurement |
|TOF|VL53LOX|V2|Precise distance|
| Display | SSD1306 OLED | 128×64px, I2C 0x3C | Real-time status display |
| Motor Driver | L298N | 2A, 12V | 4-motor PWM control |
| Motors | DC Gear Motor | 6V, 200RPM | 4WD chassis drive |
| Power Board | V2 (3.3/5/12V) | Regulated outputs | Clean power distribution |
|TOF|VL53L0X|V2|Precise distance cal|
| Chassis | 4WD Platform | Acrylic | Mechanical base |

<<<<<<< HEAD
### Car 2 — (2-Wheel Drive)
=======
### Car 2 —  (2-Wheel Drive)
>>>>>>> 07338ea0daa62cd4a166346d3483aa8157aa6346

| Component | Model | Specification | Purpose |
|-----------|-------|---------------|---------|
| Microcontroller | ESP8266 NodeMCU v2 | 80MHz, 3.3V | Main controller + ESP-NOW |
| IMU | MPU-6050 | 6-axis, I2C 0x68 | Tilt detection + crash sensing |
| IR Sensor | TCRT5000 | Digital, 3.3V | Front obstacle detection |
| Display | SSD1306 OLED | 128×64px, I2C 0x3C | Real-time status display |
| Buzzer | Active Buzzer | 3.3V, 85dB | Audio alert system |
| Motor Driver | L298N | 2A, 12V | 2-motor PWM control |
| Motors | DC Gear Motor | 6V, 200RPM | 2WD chassis drive |
| Power Board | V2 (3.3/5/12V) | Regulated outputs | Clean power distribution |
| Chassis | 2WD Platform | Acrylic | Mechanical base |

<br/>

---

##  Pin Maps

### Car 1 

```
ESP8266 NodeMCU                    Connected To
──────────────────────────────────────────────────────────────
D1  (GPIO5)   ──────────────────▶  SCL  (MPU6050 + OLED shared I2C)
D2  (GPIO4)   ──────────────────▶  SDA  (MPU6050 + OLED shared I2C)
D3  (GPIO0)   ──────────────────▶  IN1  (L298N — Motor Left  Forward)
D4  (GPIO2)   ──────────────────▶  IN2  (L298N — Motor Left  Backward)
D5  (GPIO14)  ──────────────────▶  ECHO (HC-SR04 via 1kΩ+1kΩ divider)
D6  (GPIO12)  ──────────────────▶  TRIG (HC-SR04)
D7  (GPIO13)  ──────────────────▶  IN3  (L298N — Motor Right Forward)
D8  (GPIO15)  ──────────────────▶  IN4  (L298N — Motor Right Backward)
D0  (GPIO16)  ──────────────────▶  ENA  (L298N — Left  Motor Speed PWM)
RX  (GPIO3)   ──────────────────▶  ENB  (L298N — Right Motor Speed PWM)
TX  (GPIO1)   ──────────────────▶  FREE (Serial Monitor active)

 Disconnect RX wire before uploading! Reconnect after upload.

POWER (Power Board V2):
  3.3V  ──▶  MPU6050 VCC + OLED VCC + HC-SR04 VCC
  12V   ──▶  L298N VIN (motors)
  5V    ──▶  L298N Logic VCC
  USB   ──▶  ESP8266 only
  GND   ──▶  ALL components common ground rail
```

<<<<<<< HEAD
### Car 2
=======
### Car 2 
>>>>>>> 07338ea0daa62cd4a166346d3483aa8157aa6346

```
ESP8266 NodeMCU                    Connected To
──────────────────────────────────────────────────────────────
D1  (GPIO5)   ──────────────────▶  SCL  (MPU6050 + OLED shared I2C)
D2  (GPIO4)   ──────────────────▶  SDA  (MPU6050 + OLED shared I2C)
D3  (GPIO0)   ──────────────────▶  IN1  (L298N — Motor Left  Forward)
D4  (GPIO2)   ──────────────────▶  IN2  (L298N — Motor Left  Backward)
D5  (GPIO14)  ──────────────────▶  IN3  (L298N — Motor Right Forward)
D6  (GPIO12)  ──────────────────▶  IN4  (L298N — Motor Right Backward)
D7  (GPIO13)  ──────────────────▶  ENA  (L298N — Left  Motor Speed PWM)
D8  (GPIO15)  ──────────────────▶  ENB  (L298N — Right Motor Speed PWM)
D0  (GPIO16)  ──────────────────▶  IR Sensor Signal (center front)
TX  (GPIO1)   ──────────────────▶  Buzzer Signal
RX  (GPIO3)   ──────────────────▶  FREE (not used)

 Disconnect TX wire (Buzzer) before uploading! Reconnect after upload.

POWER (Power Board V2):
  3.3V  ──▶  MPU6050 VCC + OLED VCC + IR VCC + Buzzer VCC
  12V   ──▶  L298N VIN (motors)
  5V    ──▶  L298N Logic VCC
  USB   ──▶  ESP8266 only
  GND   ──▶  ALL components common ground rail
```

<br/>

---

##  5 Safety Scenarios

Priority order — highest to lowest:

```
  [P1] Scenario 1 — Crash Detection     (MPU spike — immediate!)
  [P2] Scenario 2 — Forward Collision   (HC-SR04 critical zone)
  [P3] Scenario 3 — Shared Obstacle     (Car2 IR detected)
  [P4] Scenario 4 — Following Distance  (speed zone mirroring)
  [P5] Scenario 5 — Platoon Follow      (normal operation)
```

---

###  Scenario 1 — Forward Collision Prevention

**Trigger:** Car1 HC-SR04 distance ≤ 11cm

| Zone | Distance | Car1 Speed | Car2 Speed |
|------|----------|-----------|-----------|
| SAFE | > 50cm | 220 | 220 |
| WARNING | 20–50cm | 160 | 160 |
| DANGER | 11–20cm | 80 | 80 |
| CRITICAL | ≤ 11cm | **0 STOP** | **0 STOP** |

Car1 sends `emergency = true` via ESP-NOW → Car2 stops immediately.

---

###  Scenario 2 — Adaptive Following Distance

**Trigger:** Continuous — always active during convoy operation

Car1 broadcasts `distance_zone` every 100ms. Car2 maps zone to speed automatically, maintaining safe and proportional convoy spacing at all times.

---

###  Scenario 3 — Crash / Impact Detection

**Trigger:** `|ax_raw| > 12000` OR `|ay_raw| > 12000` on either MPU-6050

```
Tilt formula used:
  tilt = atan2(ax, sqrt(ay² + az²)) × (180 / π)

Crash threshold:
  crash = (|ax_raw| > 12000 OR |ay_raw| > 12000)
```

When either car detects a crash, it immediately stops AND sends `crash_detected = true` via ESP-NOW to the other car, which also stops. Bilateral protection.

---

###  Scenario 4 — Cooperative Obstacle Avoidance

**Trigger:** Car2 IR sensor detects obstacle (LOW signal on D0)

Car2 steers right at medium speed and simultaneously sends `ir_obstacle = true` to Car1 via ESP-NOW. Car1 receives the alert and mirrors the avoidance maneuver. Both vehicles avoid the obstacle together.

---

###  Scenario 5 — Autonomous Platoon Follow

**Trigger:** Default — no obstacles, no emergencies, ESP-NOW link active

Car2 mirrors Car1's exact speed and direction. Both OLEDs display `PLATOON RUNNING`. This is the baseline operating mode of the convoy system.

<br/>

---

## 📡 ESP-NOW Protocol

### Why ESP-NOW over WiFi?

| Feature | ESP-NOW | WiFi UDP | Bluetooth |
|---------|---------|----------|-----------|
| Latency | **~1ms** | 50–200ms | 10–50ms |
| Range | **~200m** | ~50m | ~10m |
| Router needed | **No** | Yes | No |
| Pairing | **MAC address** | SSID/Password | Pairing code |
| Power consumption | **Very Low** | High | Medium |
| Max nodes | 20 peers | Unlimited | 7 |

### Bidirectional Data Flow

```
  CAR 1 (COMBO role)               CAR 2 (COMBO role)
  ──────────────────               ──────────────────

  Every 100ms SEND:                Every 100ms SEND:
  ┌─────────────────┐              ┌─────────────────┐
  │ distance_cm     │──ESP-NOW───▶ │ ir_obstacle     │
  │ distance_zone   │              │ crash_detected  │
  │ tilt_angle      │◀──ESP-NOW─── │ tilt_angle      │
  │ crash_detected  │              │ speed           │
  │ emergency       │              │ direction       │
  │ active_scenario │              │ active_scenario │
  └─────────────────┘              └─────────────────┘
```

### ESP-NOW Role Used

```cpp
// Both cars use COMBO role for full bidirectional:
esp_now_set_self_role(ESP_NOW_ROLE_COMBO);

// Both cars register each other as peers:
esp_now_add_peer(peerMAC, ESP_NOW_ROLE_COMBO, 1, NULL, 0);

// Both register send + receive callbacks:
esp_now_register_send_cb(onSent);
esp_now_register_recv_cb(onReceive);
```

<br/>

---

## 📁 Repository Structure

```
V2V_-Communication-Multi-Sensor-Fusion-/
│
├── 📂 car1_leader/                  ← Phase 9 — Car1 original
│   ├── src/main.cpp
│   └── platformio.ini
│
├── 📂 car2_follower/                ← Phase 10 — Car2 original
│   ├── src/main.cpp
│   └── platformio.ini
│
├── 📂 car1_bidirectional/           ← Phase 11 — Car1 bidirectional
│   ├── src/main.cpp                    All 5 scenarios + receives Car2
│   └── platformio.ini
│
├── 📂 car2_bidirectional/           ← Phase 11 — Car2 bidirectional
│   ├── src/main.cpp                    All 5 scenarios + sends to Car1
│   └── platformio.ini
│
├── 📂 dashboard/                    ← Python live dashboards
│   ├── car1_dashboard.py               
│   └── car2_dashboard_v2.py          
│
├── 📂 tests/                        ← Component test sketches (Phases 1–8)
│   ├── test_mpu6050/
│   ├── test_oled/
│   ├── test_espnow/
│   ├── test_hcsr04/
│   ├── test_ir_sensors/
│   ├── test_l298n/
│   └── test_buzzer/
│
├── 📂 docs/
│   ├── wiring_guide.md
│   ├── troubleshooting.md
│   └── hardware_components.md
│
└── README.md
```

<br/>

---

##  Getting Started

### Prerequisites

**Hardware required:**
- 2× ESP8266 NodeMCU v2
- 2× MPU-6050 (6-axis IMU)
- 2× SSD1306 OLED (128×64, I2C)
- 1× HC-SR04 Ultrasonic Sensor
- 1× IR Obstacle Sensor
- 2× L298N Motor Driver
- 1× Active Buzzer
- 2× Power Board V2 (3.3V/5V/12V)
- 6× DC Gear Motors (4 for Car1, 2 for Car2)
- 2× 12V Battery

**Software required:**
```bash
# PlatformIO (VS Code extension) — for ESP8266 code
# Python 3.x — for dashboards
pip install pyserial matplotlib numpy
```

---

### Step 1 — Clone Repository

```bash
git clone https://github.com/deepakmurugan578-beep/V2V_-Communication-Multi-Sensor-Fusion-.git
cd V2V_-Communication-Multi-Sensor-Fusion-
```

---

### Step 2 — Discover MAC Addresses

```
1. Open tests/test_espnow/ in PlatformIO
2. Upload to Car1 ESP8266
3. Open Serial Monitor (115200 baud)
4. Note Car1 MAC address
5. Repeat for Car2 ESP8266
```

---

### Step 3 — Update MAC Addresses

In `car1_bidirectional/src/main.cpp`:
```cpp
uint8_t car2MAC[] = {0xC8, 0xC9, 0xA3, 0x66, 0x97, 0xD1}; // ← your Car2 MAC
```

In `car2_bidirectional/src/main.cpp`:
```cpp
uint8_t car1MAC[] = {0xC4, 0x5B, 0xBE, 0x71, 0x31, 0x47}; // ← your Car1 MAC
```

---

### Step 4 — Upload Car 1

```
1. Open car1_bidirectional/ in PlatformIO
2. Disconnect RX wire from Car1
3. Click Upload → wait for SUCCESS
4. Reconnect RX wire
```

### Step 5 — Upload Car 2

```
1. Open car2_bidirectional/ in PlatformIO
2. Disconnect TX wire (Buzzer) from Car2
3. Click Upload → wait for SUCCESS
4. Reconnect TX wire
```

---

### Step 6 — Run Dashboards

```bash
cd dashboard

# Car 1 dashboard
python car1_dashboard.py

# Car 2 dashboard (new terminal)
python car2_dashboard_v2.py
```

>  Close PlatformIO Serial Monitor before running Python dashboards.

---

### Step 7 — Power On Convoy

```
1. Power Car1 via USB or battery
2. Power Car2 via USB or battery
3. Both OLEDs show "READY!"
4. Car1 OLED shows [C2:OK] when linked
5. Car2 OLED shows [C1:OK] when linked
6. Convoy is live!
```

<br/>

---

##  Python Dashboards

### Car 1 — Leader Dashboard (Orange Theme)

Live panels updated every 120ms:
- Arc gauges: Distance, Tilt, Motor Speed
- Direction indicator with animated arrow
- Message type pill: NORMAL / WARNING / EMERGENCY
- Zone bar: SAFE / WARNING / DANGER / CRITICAL
- Alert pills: Crash + Braking
- 4 expert live graphs with gradient fill and glow lines

### Car 2 — Follower Dashboard (Cyan Theme)

Live panels updated every 120ms:
- Arc gauges: Car1 Distance, Car1 Speed, Car2 Tilt, Car2 Speed
- IR sensor panel with detection indicator
- Zone bar reflecting Car1 distance zone
- Alert pills: Link status + Emergency
- 4 expert live graphs

<br/>

---

##  Data Packets

### Car1Data struct (Car1 → Car2)

```cpp
struct Car1Data {
  uint8_t  vehicle_id;       // Always 1
  uint8_t  msg_type;         // 1=normal 2=warning 3=emergency
  uint32_t timestamp;        // millis() for packet age
  int16_t  ax, ay, az;       // MPU6050 raw accelerometer
  int16_t  gx, gy, gz;       // MPU6050 raw gyroscope
  float    tilt_angle;       // Degrees — atan2 formula
  bool     crash_detected;   // |ax| or |ay| > 12000
  uint16_t distance_cm;      // HC-SR04 (999 = no reading)
  uint8_t  distance_zone;    // 0=safe 1=warn 2=danger 3=critical
  bool     obstacle_near;    // distance <= DIST_DANGER
  uint8_t  speed;            // Motor PWM 0–255
  int8_t   direction;        // -1=left 0=forward 1=right
  bool     braking;
  bool     reversing;
  bool     platoon_active;
  bool     emergency;
  uint8_t  active_scenario;  // 1–5
};
```

### Car2Data struct (Car2 → Car1)

```cpp
struct Car2Data {
  uint8_t  vehicle_id;       // Always 2
  uint8_t  msg_type;
  uint32_t timestamp;
  int16_t  ax, ay, az;       // MPU6050 raw
  float    tilt_angle;
  bool     crash_detected;
  bool     ir_obstacle;      // IR sensor triggered
  uint8_t  speed;
  int8_t   direction;
  bool     braking;
  bool     platoon_active;
  bool     emergency;
  uint8_t  active_scenario;
};
```

<br/>

---

##  Results & Performance

### Communication

| Metric | Value |
|--------|-------|
| ESP-NOW average latency | ~1ms |
| Packet rate | 10 Hz (100ms interval) |
| Effective range (LOS) | ~200m |
| Packet loss (≤ 5m) | < 1% |
| Connection timeout | 2000ms |
| Boot time (both cars) | < 3 seconds |

### Sensor Performance

| Sensor | Range | Accuracy | Rate |
|--------|-------|----------|------|
| HC-SR04 | 2–400cm | ±3mm | 10Hz |
| MPU-6050 Tilt | ±90° | ±0.5° | 10Hz |
| MPU-6050 Crash | Threshold-based | Immediate | 10Hz |
| IR Sensor | 2–30cm | Digital ON/OFF | 10Hz |

### Scenario Response Times

| Scenario | Trigger → Action | Status |
|----------|-----------------|--------|
| Scenario 1 — Collision | < 200ms | ✅ Tested |
| Scenario 2 — Following | Continuous | ✅ Tested |
| Scenario 3 — Crash | < 100ms | ✅ Tested |
| Scenario 4 — IR Obstacle | < 150ms | ✅ Tested |
| Scenario 5 — Platoon | Continuous | ✅ Tested |

<br/>

---

##  Troubleshooting

| Problem | Cause | Solution |
|---------|-------|----------|
| `COM3 PermissionError` | RX/TX wire connected during upload | Disconnect RX (Car1) or TX (Car2) first |
| MPU6050 tilt always `0.0` | Wrong tilt formula | Use `atan2(ax, sqrt(ay²+az²))` |
| OLED blank | Wrong I2C address | Confirm address is `0x3C` |
| ESP-NOW `LINK:WAIT` forever | Wrong MAC address | Re-run test_espnow to get correct MACs |
| Car2 IR false triggers | Pin conflict at boot | Use D0 only — never RX pin for IR |
| Buzzer beeps at random | TX shared with Serial | Call `Serial.end()` before using TX |
| Zone boundary incorrect | DIST_CRITICAL value | Use `11` not `10` for critical threshold |
| `pip` not recognized | Python PATH issue | Use `python -m pip install` |
| A0 IR not working | A0 max input is 1V | 3.3V IR output exceeds A0 limit — use D0 |
| PlatformIO wrong folder | Nested project folder | Open parent folder, not `src/` directly |

> See `docs/troubleshooting.md` for extended troubleshooting guide.

<br/>

---

##  Future Work

- [ ] **PID Speed Control** — Replace fixed speed zones with proportional distance-based speed regulation
- [ ] **3+ Vehicle Convoy** — Extend ESP-NOW mesh to handle Car3, Car4 with forwarding
- [ ] **GPS Integration** — Absolute position sharing between convoy vehicles
- [ ] **Web Dashboard** — Browser-based real-time UI replacing Python desktop app
- [ ] **ML Crash Prediction** — Train a lightweight model on MPU-6050 data to predict impacts before they happen
- [ ] **Custom PCB** — Replace breadboard wiring with purpose-built PCB for reliability
- [ ] **OTA Firmware Updates** — Simultaneous over-the-air updates for both vehicles
- [ ] **Reverse Gear Scenario** — Bidirectional platoon movement

<br/>

---

##  References

- Espressif Systems — [ESP-NOW Protocol Docs](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/network/esp_now.html)
- InvenSense — [MPU-6050 Datasheet](https://invensense.tdk.com/products/motion-tracking/6-axis/mpu-6050/)
- Adafruit — [SSD1306 OLED Library](https://github.com/adafruit/Adafruit_SSD1306)
- PlatformIO — [ESP8266 Arduino Framework](https://docs.platformio.org/en/latest/platforms/espressif8266.html)

<br/>

---

##  Author

<div align="center">

### Deepak M

**Embedded Systems & IoT Engineer**

V2V Multi-Sensor Fusion Communication System
Built from scratch — component testing through full bidirectional integration

---

[![GitHub](https://img.shields.io/badge/GitHub-deepakmurugan578--beep-black?style=for-the-badge&logo=github)](https://github.com/deepakmurugan578-beep)

---

*This project was developed as a  demonstration of real-time embedded communication,*
*multi-sensor fusion, and autonomous vehicle systems engineering.*

<br/>

```
"The best way to predict the future is to build it."
```

<br/>

 If this project helped you, please star the repository!

</div>

<br/>

---

<div align="center">

**MIT License** · Copyright © 2026 Deepak M

*Built with ESP8266 · ESP-NOW · PlatformIO · Python*

</div>
