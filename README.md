# V2V_-Communication-Multi-Sensor-Fusion-
A Real Time (Vehicle to Vehicles ) V2V Convoy   using  ESP8266   ESP-NOW Protocol  -  Leader  car broadcasts sensor data wirelessly, follower car mirrors every movement autonomously.


> A real-time Vehicle-to-Vehicle (V2V) communication system 
> built with ESP8266 and ESP-NOW protocol. 
> Car 1 (leader) broadcasts sensor data wirelessly — 
> Car 2 (follower) mirrors every movement autonomously.

## 📽️ Demo

> *(Add your demo video or GIF here)*

## 🏗️ System Architecture
```
CAR 1 LEADER (4-wheel)          CAR 2 FOLLOWER (2-wheel)
┌─────────────────────┐         ┌─────────────────────┐
│  ESP8266            │         │  ESP8266            │
│  MPU6050            │ESP-NOW  │  MPU6050            │
│  HC-SR04           │────────▶│  OLED Display       │
│  4x IR Sensors      │         │  L298N Motors       │
│  L298N Motors       │         └─────────────────────┘
└─────────────────────┘

##  Hardware Requirements

 Car 1 — Leader (4-wheel)
|Component |
|----------|
| ESP8266 NodeMCU|
| MPU6050 |
| HC-SR04|
| IR Sensor |
| L298N Motor Driver | 
| 4-Wheel Chassis |
| 12V Battery | 

### Car 2 — Follower (2-wheel)
| Component |
|-----------|
| ESP8266 NodeMCU |
|VL53L0X|
| MPU6050 | 
| OLED 128x64 |
| L298N Motor Driver | 
| 2-Wheel Chassis | 


##  ESP-NOW Data Packet
```cpp
struct V2VData {
  int16_t  ax, ay, az;       // accelerometer raw
  int16_t  gx, gy, gz;       // gyroscope raw
  float    tilt_angle;       // calculated tilt
  uint16_t distance_cm;      // HC-SR04 distance
  bool     obstacle_near;    // obstacle < 20cm
  bool     ir_front_left;    // IR front left
  bool     ir_front_right;   // IR front right
  bool     ir_rear_left;     // IR rear left
  bool     ir_rear_right;    // IR rear right
  uint8_t  speed;            // motor speed 0-255
  int8_t   direction;        // -1 left 0 straight 1 right
  bool     braking;          // emergency brake flag
  bool     reversing;        // reverse flag
};
```

---

##  Wiring

### Car 1 Pin Connections
| Sensor/Module | ESP8266 Pin |
|---------------|-------------|
| MPU6050 SDA | D2 |
| MPU6050 SCL | D1 |
| HC-SR04 TRIG | D6 |
| HC-SR04 ECHO | D5 |
| IR Front Left | D3 |
| IR Front Right | D4 |
| IR Rear Left | D7 |
| IR Rear Right | D8 |
| L298N IN1 | D0 |
| L298N IN2 | D9 |
| L298N IN3 | D10 |
| L298N IN4 | D11 |
| L298N ENA | D12 |
| L298N ENB | D13 |

### Car 2 Pin Connections
| Module | ESP8266 Pin |
|--------|-------------|
| MPU6050 SDA | D2 |
| MPU6050 SCL | D1 |
| OLED SDA | D2 |
| OLED SCL | D1 |
| L298N IN1 | D3 |
| L298N IN2 | D4 |
| L298N IN3 | D5 |
| L298N IN4 | D6 |
| L298N ENA | D7 |
| L298N ENB | D8 |

---

## 🚀 Getting Started

### Prerequisites
- VS Code with PlatformIO extension installed
- Git installed on your PC

### Installation
```bash
# Clone the repository
git clone https://github.com/YOUR_USERNAME/v2v-esp8266-convoy.git

# Open in VS Code
cd v2v-esp8266-convoy
code .
```

### Step 1 — Find Receiver MAC Address
```bash
# Open tests/test_espnow in VS Code
# Upload to Car2 ESP8266
# Note MAC address from Serial Monitor
```

### Step 2 — Update Sender MAC
```cpp
// In car1_leader/src/main.cpp
uint8_t receiverMAC[] = {0xXX, 0xXX, 0xXX, 0xXX, 0xXX, 0xXX};
```

### Step 3 — Upload Car1 Code
```bash
# Open car1_leader in VS Code
# Upload to 4-wheel ESP8266
```

### Step 4 — Upload Car2 Code
```bash
# Open car2_follower in VS Code
# Upload to 2-wheel ESP8266
```

### Step 5 — Power Both Cars
```bash
# Connect 12V battery to both cars
# Car2 OLED shows Car1 live data
# Car2 mirrors Car1 movement!
```

---

## 🧪 Sensor Tests

Run individual sensor tests before deploying:
```bash
tests/test_mpu6050/      # Test MPU6050 alone
tests/test_hcsr04/       # Test HC-SR04 alone
tests/test_ir_sensors/   # Test all 4 IR sensors
tests/test_l298n_motors/ # Test motor directions
tests/test_espnow/       # Test wireless link
tests/test_oled/         # Test OLED display
```

---

## 📊 Project Phases

- [x] Phase 1 — MPU6050 sensor test
- [x] Phase 2 — OLED display test
- [x] Phase 3 — ESP-NOW communication
- [ ] Phase 4 — HC-SR04 test
- [ ] Phase 5 — IR sensor test
- [ ] Phase 6 — L298N motor test
- [ ] Phase 7 — Car1 full integration
- [ ] Phase 8 — Car2 full integration
- [ ] Phase 9 — V2V convoy test

---

## 🐛 Troubleshooting

See [docs/troubleshooting.md](docs/troubleshooting.md) for common issues.

---

## 📄 License

MIT License — see [LICENSE](LICENSE) for details.

---

## 👨‍💻 Author

**M Deepak**
- Location: Tambaram, Tamil Nadu, India
- Project: V2V ESP8266 Convoy System

---

## 🌟 Show Your Support

Give a ⭐ if this project helped you!
