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
