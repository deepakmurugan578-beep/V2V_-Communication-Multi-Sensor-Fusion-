# Troubleshooting Guide

**Project:** V2V Multi-Sensor Fusion Communication System  
**Author:** Deepak M | 2025

---

## Upload Issues

| Problem | Cause | Fix |
|---------|-------|-----|
| Upload fails on Car 1 | RX wire connected | Disconnect RX, upload, reconnect |
| Upload fails on Car 2 | TX (buzzer) wire connected | Disconnect TX, upload, reconnect |
| `COM3 PermissionError` | Serial Monitor is open | Close Serial Monitor first |
| Port not found | Wrong COM port | Check Device Manager, update platformio.ini |

---

## ESP-NOW Issues

| Problem | Cause | Fix |
|---------|-------|-----|
| LINK shows WAIT forever | Wrong MAC address | Run test_espnow sketch to get correct MACs |
| LINK shows WAIT forever | Different WiFi channels | Both must use channel 1 |
| Packets lost frequently | Cars too far apart | Move within 5m for initial testing |
| Car 2 not reacting | onReceive not registered | Check esp_now_register_recv_cb in setup() |

---

## Sensor Issues

| Problem | Cause | Fix |
|---------|-------|-----|
| MPU6050 tilt always 0.0 | Wrong formula | Use `atan2(ax, sqrt(ay²+az²))` |
| MPU6050 not found | Wrong I2C address | Confirm ADO pin is at GND → address 0x68 |
| OLED blank | Wrong I2C address | Run I2C scanner, verify 0x3C |
| HC-SR04 always 999cm | Voltage divider missing | Check 1kΩ+1kΩ on ECHO pin |
| HC-SR04 noisy readings | Single reading | Take 3 readings, use median |
| IR false triggers on Car 2 | Wrong pin | Use D0 only — never RX (GPIO3) |
| IR sensor always LOW | Wiring issue | Check VCC=3.3V, GND=rail, Signal=D0 |
| A0 IR not working | A0 voltage limit exceeded | A0 max is 1V — use D0 instead |

---

## Motor Issues

| Problem | Cause | Fix |
|---------|-------|-----|
| Motors not spinning | L298N not powered | Check 12V on VIN and 5V on VCC |
| One motor wrong direction | OUT wires swapped | Swap OUT1/OUT2 or OUT3/OUT4 |
| Motors jitter at low speed | PWM too low | Minimum is SPEED_SLOW = 80 |
| Car drives in a circle | Motor speeds unequal | Tune ENA/ENB PWM values |
| No speed control | ENA/ENB jumpers on | Remove jumpers from L298N ENA/ENB |

---

## Buzzer Issues

| Problem | Cause | Fix |
|---------|-------|-----|
| Buzzer beeps at boot | TX active during Serial.begin() | Call `Serial.end()` before using TX as output |
| Buzzer always ON | pinMode not set | Add `pinMode(1, OUTPUT)` after Serial.end() |
| Buzzer beeps during upload | TX is programming line | Disconnect buzzer wire before upload |
| No sound at all | Wrong pin | Confirm buzzer signal is on TX (GPIO1) |

---

## Python Dashboard Issues

| Problem | Cause | Fix |
|---------|-------|-----|
| `PermissionError` on COM port | Serial Monitor still open | Close PlatformIO Serial Monitor |
| No data appearing | Wrong COM port | Update port in dashboard script |
| Graph not updating | Serial format wrong | Verify `/*...,*/` format in firmware output |
| Dashboard crashes on start | Missing Python libraries | Run `pip install pyserial matplotlib numpy` |

---

## Power Issues

| Problem | Cause | Fix |
|---------|-------|-----|
| ESP8266 keeps rebooting | Power insufficient | Use dedicated Power Board, not USB alone for motors |
| Motors stop randomly | Battery voltage sag | Charge 12V battery |
| Sensors give wrong readings | Noisy power supply | Add 100µF capacitor across Power Board output |
| Components get hot | Wrong voltage rail | Double-check 3.3V rail — not 5V — to sensors |

---

## Common Pin Mistakes

| Mistake | Result | Correct Pin |
|---------|--------|------------|
| IR sensor on RX (GPIO3) | False triggers at boot | D0 (GPIO16) |
| IR sensor on A0 | Possible ADC damage | D0 (GPIO16) |
| HC-SR04 ECHO direct to D5 | GPIO damage from 5V | D5 via 1kΩ+1kΩ divider |
| Buzzer on RX instead of TX | No sound, boot issues | TX (GPIO1) only |
| USB + 12V battery together | Possible ESP8266 damage | One power source at a time |
| Missing common GND | Sensors read garbage | Connect all GNDs to one rail |
