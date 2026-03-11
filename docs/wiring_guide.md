# Wiring Guide

**Project:** V2V Multi-Sensor Fusion Communication System  
**Author:** Deepak M | 2025

---

## Power Setup (Both Cars)

| Rail | Voltage | Powers |
|------|---------|--------|
| Rail 1 | 3.3V | MPU6050, OLED, IR Sensor, Buzzer |
| Rail 2 | 5V | L298N logic VCC |
| Rail 3 | 12V | L298N motor VIN |
| USB | 5V in | ESP8266 only |
| GND | 0V | All components — common rail |

>  Never connect USB and 12V battery at the same time.  
>  All GNDs must share one common ground rail.

---

## HC-SR04 Voltage Divider (Car 1 Only)

ECHO outputs 5V — ESP8266 GPIO max is 3.3V. Use two 1kΩ resistors:

```
HC-SR04 ECHO ──[1kΩ]──┬──[1kΩ]──  GND
                       └──  ESP8266 D5
```

---

## Car 1 — Leader

>  Disconnect **RX** wire before uploading. Reconnect after.

| ESP8266 Pin | Connected To | Purpose |
|-------------|-------------|---------|
| D1 (GPIO5) | MPU6050 SCL + OLED SCL | I2C clock |
| D2 (GPIO4) | MPU6050 SDA + OLED SDA | I2C data |
| D3 (GPIO0) | L298N IN1 | Left motor forward |
| D4 (GPIO2) | L298N IN2 | Left motor backward |
| D5 (GPIO14) | HC-SR04 ECHO via divider | Distance echo |
| D6 (GPIO12) | HC-SR04 TRIG | Distance trigger |
| D7 (GPIO13) | L298N IN3 | Right motor forward |
| D8 (GPIO15) | L298N IN4 | Right motor backward |
| D0 (GPIO16) | L298N ENA | Left motor PWM |
| RX (GPIO3) | L298N ENB | Right motor PWM  |
| TX (GPIO1) | FREE | Serial Monitor |

### MPU6050

| Pin | Connect To |
|-----|------------|
| VCC | 3.3V |
| GND | GND rail |
| SCL | D1 |
| SDA | D2 |
| ADO | GND (address 0x68) |

### OLED SSD1306

| Pin | Connect To |
|-----|------------|
| VCC | 3.3V |
| GND | GND rail |
| SCL | D1 (shared) |
| SDA | D2 (shared) |

### HC-SR04

| Pin | Connect To |
|-----|------------|
| VCC | 3.3V |
| GND | GND rail |
| TRIG | D6 |
| ECHO | D5 via 1kΩ+1kΩ divider |

### L298N (4WD)

| Pin | Connect To |
|-----|------------|
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

## Car 2 — Follower

>  Disconnect **TX** (buzzer wire) before uploading. Reconnect after.  
>  Do NOT use A0 for IR sensor — max input 1V, IR outputs 3.3V.

| ESP8266 Pin | Connected To | Purpose |
|-------------|-------------|---------|
| D1 (GPIO5) | MPU6050 SCL + OLED SCL | I2C clock |
| D2 (GPIO4) | MPU6050 SDA + OLED SDA | I2C data |
| D3 (GPIO0) | L298N IN1 | Left motor forward |
| D4 (GPIO2) | L298N IN2 | Left motor backward |
| D5 (GPIO14) | L298N IN3 | Right motor forward |
| D6 (GPIO12) | L298N IN4 | Right motor backward |
| D7 (GPIO13) | L298N ENA | Left motor PWM |
| D8 (GPIO15) | L298N ENB | Right motor PWM |
| D0 (GPIO16) | IR Sensor signal | Obstacle detection |
| TX (GPIO1) | Buzzer | Audio alerts  |
| RX (GPIO3) | FREE | Not connected |

### MPU6050

Same as Car 1 — VCC→3.3V, GND→rail, SCL→D1, SDA→D2, ADO→GND.

### OLED SSD1306

Same as Car 1 — VCC→3.3V, GND→rail, SCL→D1, SDA→D2.

### IR Sensor

| Pin | Connect To |
|-----|------------|
| VCC | 3.3V |
| GND | GND rail |
| Signal | D0 |

### Buzzer

| Pin | Connect To |
|-----|------------|
| VCC | 3.3V |
| GND | GND rail |
| Signal | TX (GPIO1) |

### L298N (2WD)

| Pin | Connect To |
|-----|------------|
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

## Upload Steps

**Car 1:**
1. Disconnect RX wire
2. Upload in PlatformIO
3. Reconnect RX wire

**Car 2:**
1. Disconnect TX wire (Buzzer)
2. Upload in PlatformIO
3. Reconnect TX wire
