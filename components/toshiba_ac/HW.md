```
Vcc (+5V)
    |
    R1 (100Ω)
    |
   IR LED (Anode)
    |
   IR LED (Cathode)
    |
 Collector (BC327)
    |
 Emitter (BC327) → GND
    |
 Base (BC327) ← R2 (100Ω) ← Signal (38kHz from ESP32)
```

**IR LED Circuit Connections:**

| Component | Connection |
|-----------|-----------|
| Power Supply | Vcc (+5V) |
| Resistor R1 | 100Ω (current limiting) |
| IR LED Anode | Connected to R1 |
| IR LED Cathode | Connected to BC327 Collector |
| BC327 Emitter | Connected to GND |
| BC327 Base | Connected to R2 (1kΩ) and Signal (38kHz) |
