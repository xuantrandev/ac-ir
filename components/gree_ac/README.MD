# gree_ac — ESPHome IR Climate Component

Custom ESPHome component that decodes and encodes the IR protocol of Gree AC remote controls, exposing a `climate` entity to Home Assistant.

---

## Protocol Overview

Reverse-engineered from raw IR captures (`components/gree_ac/raw/log`).

| Parameter | Value |
|---|---|
| Carrier | 38 kHz |
| Encoding | Pulse-distance, **LSB-first** per byte |
| Header | 9000 µs mark + 4500 µs space |
| Bit `1` | 660 µs mark + 1625 µs space |
| Bit `0` | 660 µs mark + 537 µs space |
| Inter-frame gap | ~19800 µs |

Each command consists of **two frames** sent back-to-back.

---

### Frame 1 — Settings (32 data bits + 3 trailing bits)

Header present. 4 data bytes followed by 3 fixed trailing bits `{0, 1, 0}`.

```
Byte  Bits      Description
  0   [2:0]     Mode  (0=auto, 1=cool, 2=dry, 3=fan_only, 4=heat)
      [3]       Power (0=OFF, 1=ON)
      [5:4]     Fan   (0=auto, 1=low, 2=med, 3=high)
      [6]       Swing (0=off, 1=on)
      [7]       0 (constant)
  1   [3:0]     Temperature − 16  (16–30 °C)
      [7:4]     0 (constant)
  2   —         0xE0 for cool/dry · 0x60 for heat/fan/auto · 0xA0 for power-off
  3   —         0x50 (constant)
```

**Mode values (Byte 0 bits[2:0]):**

| Value | Mode |
|---|---|
| `0` | Auto ✓ |
| `1` | Cool ✓ |
| `2` | Dry ✓ |
| `3` | Fan only ✓ |
| `4` | Heat ✓ |

**Fan speed values (Byte 0 bits[5:4]):**

| Value | Speed |
|---|---|
| `0` | Auto ✓ |
| `1` | Low ✓ |
| `2` | Medium ✓ |
| `3` | High ✓ |

**Power OFF:** `Byte 0 bit[3] = 0`; Byte 2 becomes `0xA0`. All other fields preserve last state.

**Temperature range:** 16–30 °C in 1 °C steps.

---

### Frame 2 — Checksum footer (32 bits, **no header**)

No 9000 µs header — starts immediately with bit pulses.

```
Byte  Value        Description
  0   0x01 / 0x00  Swing echo: 0x01 if swing ON, 0x00 if swing OFF
  1   0x40         Constant
  2   0x00         Constant
  3   CS << 4      Checksum nibble in upper nibble; lower nibble = 0
```

**Checksum formula** (verified against all 10 captured commands):

```
CS = ((F1[0] & 0xF) + (F1[1] & 0xF) + (F1[2] & 0xF) + (F1[3] & 0xF) + 0xE) & 0xF
Byte 3 = CS << 4
```

---

## Files

```
components/gree_ac/
├── __init__.py       ESPHome package marker
├── climate.py        ESPHome codegen (config schema + C++ variable setup)
├── gree_ac.h         C++ class declaration
├── gree_ac.cpp       C++ implementation (encode / decode / control)
└── example.yaml      Full example ESPHome configuration
```

---

## Usage

### 1. Reference the component

```yaml
external_components:
  - source:
      type: local
      path: components      # folder that contains gree_ac/
    components: [gree_ac]
```

### 2. Declare IR hardware

```yaml
remote_transmitter:
  id: ir_tx
  pin:
    number: GPIO8
    mode: OUTPUT
    inverted: true
  carrier_duty_percent: 50%

remote_receiver:
  id: ir_rx
  pin:
    number: GPIO21
    mode: INPUT_PULLUP
    inverted: true
  tolerance: 25%
  dump: raw
```

### 3. Add the climate entity

```yaml
climate:
  - platform: gree_ac
    name: "Gree AC"
    receiver_id: ir_rx
    transmitter_id: ir_tx
```

See [example.yaml](example.yaml) for a complete configuration.

---

## Supported Features

| Feature | Supported |
|---|---|
| Cool / Heat / Dry / Fan-only / Auto | ✓ |
| Temperature 16–30 °C (1 °C steps) | ✓ |
| Fan speed: auto / low / medium / high | ✓ |
| Vertical swing on / off | ✓ |
| Receive IR from physical remote | ✓ |
| Transmit IR to AC unit | ✓ |
| State persistence across reboots | ✓ |
