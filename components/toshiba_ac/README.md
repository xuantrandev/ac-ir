# toshiba_ac — ESPHome IR Climate Component

Custom ESPHome component that decodes and encodes the IR protocol of Toshiba AC remote controls, exposing a `climate` entity to Home Assistant.

---

## Protocol Overview

Reverse-engineered from raw IR captures (`raw/toshiba/log`).

| Parameter | Value |
|---|---|
| Carrier | 38 kHz |
| Encoding | Pulse-distance (LSB-first per byte, MSB-first bit order) |
| Header | 4500 µs mark + 4500 µs space |
| Bit `1` | 560 µs mark + 1600 µs space |
| Bit `0` | 560 µs mark + 560 µs space |
| Repeat gap | 5500 µs |

Every command is sent **twice** (original frame + identical repeat).

### 9-byte Settings Packet (mode / temperature / fan)

```
Byte  Value    Description
  0   0xF2     Toshiba device ID
  1   0x0D     Toshiba device ID
  2   0x03     Command: settings
  3   0xFC     Constant
  4   0x01     Constant
  5   TT00     TT = (temp_celsius - 17) << 4   (fan-only: temp ignored, uses 0x50)
  6   FFMM     FF = fan nibble, MM = mode nibble
  7   0x00     Constant
  8   CHK      XOR checksum of bytes 0–7
```

**Mode nibbles (byte 6 low):**

| Nibble | Mode |
|---|---|
| `0x0` | Auto |
| `0x1` | Cool ✓ |
| `0x2` | Dry ✓ |
| `0x3` | Heat * |
| `0x4` | Fan only ✓ |
| `0x7` | Off ✓ |

**Fan speed nibbles (byte 6 high):**

| Nibble | Speed |
|---|---|
| `0x0` | Auto ✓ |
| `0x4` | Low (fan 1) ✓ |
| `0x6` | Middle (fan 2) ✓ |
| `0x8` | Medium (fan 3) ✓ |
| `0xA` | High (fan 4) ✓ |

✓ = verified from captures &nbsp;|&nbsp; \* = inferred from protocol structure

**Temperature range:** 17–30 °C in 1 °C steps.

### 7-byte Swing Packet

```
Byte  Value    Description
  0   0xF2     Toshiba device ID
  1   0x0D     Toshiba device ID
  2   0x01     Command: swing
  3   0xFE     Constant
  4   0x21     Constant
  5   0x01     Swing vertical ON  ✓
      0x02     Swing vertical OFF ✓
  6   CHK      XOR checksum of bytes 0–5
```

Swing state is maintained by the AC unit between commands. A swing packet is only sent when the swing state **changes**.

---

## Files

```
components/toshiba_ac/
├── __init__.py       ESPHome package marker
├── climate.py        ESPHome codegen (config schema + C++ variable setup)
├── toshiba_ac.h      C++ class declaration
├── toshiba_ac.cpp    C++ implementation (encode / decode / control)
└── example.yaml      Full example ESPHome configuration
```

---

## Usage

### 1. Reference the component

In your ESPHome YAML, point `external_components` at the folder containing `toshiba_ac/`:

```yaml
external_components:
  - source:
      type: local
      path: components      # folder that contains toshiba_ac/
    components: [toshiba_ac]
```

### 2. Declare IR hardware

```yaml
remote_transmitter:
  id: ir_tx
  pin:
    number: GPIO20
    mode: OUTPUT_OPEN_DRAIN
    inverted: true
  carrier_duty_percent: 50%

remote_receiver:
  id: ir_rx
  pin:
    number: GPIO21
    mode: INPUT_PULLUP
    inverted: true
  tolerance: 50%
```

### 3. Add the climate entity

```yaml
climate:
  - platform: toshiba_ac
    name: "Toshiba AC"
    receiver_id: ir_rx
    transmitter_id: ir_tx
```

See [example.yaml](example.yaml) for a complete configuration.

---

## Supported Features

| Feature | Supported |
|---|---|
| Cool / Heat / Dry / Fan-only / Auto | ✓ |
| Temperature 17–30 °C (1 °C steps) | ✓ |
| Fan speed: auto / low / middle / medium / high | ✓ |
| Vertical swing on / off | ✓ |
| Receive IR from physical remote | ✓ |
| Transmit IR to AC unit | ✓ |
| State persistence across reboots | ✓ |

---

## Known Limitations

- **Heat mode** — included in the protocol map but not verified with a physical capture. Use with caution.
- **Swing** — both ON (`0x01`) and OFF (`0x02`) values are now verified from captures (byte 5 of the 7-byte swing packet).
