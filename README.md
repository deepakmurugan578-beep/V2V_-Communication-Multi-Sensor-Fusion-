# V2V_-Communication-Multi-Sensor-Fusion-
A Real Time (Vehicle to Vehicles ) V2V Convoy   using  ESP8266   ESP-NOW Protocol  -  Leader  car broadcasts sensor data wirelessly, follower car mirrors every movement autonomously.

> A real-time Vehicle-to-Vehicle (V2V) communication system 
> built with ESP8266 and ESP-NOW protocol. 
> Car 1 (leader) broadcasts sensor data wirelessly — 
> Car 2 (follower) mirrors every movement autonomously.

##  Demo

> *(Add your demo video or GIF here)*

##  System Architecture
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
