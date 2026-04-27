#pragma once

#include "esphome/core/component.h"
#include "esphome/components/climate/climate.h"
#include "esphome/components/remote_base/remote_base.h"

namespace esphome {
namespace toshiba_ac {

/**
 * Toshiba AC IR Climate Component
 *
 * Implements the Toshiba AC 9-byte IR protocol reverse-engineered from
 * raw remote captures. Sends and receives IR signals to control the AC.
 *
 * === Protocol (MSB first, pulse-distance encoding) ===
 *
 * Header: 4500µs mark + 4500µs space
 * Bit '1': 560µs mark + 1600µs space
 * Bit '0': 560µs mark +  560µs space
 * Terminal burst: 560µs mark
 * Repeat gap: 5500µs space
 * Carrier: 38 kHz
 *
 * All packets are sent twice (original + repeat).
 *
 * --- 9-byte Settings Packet (72 bits) ---
 * Byte 0: 0xF2   Toshiba device ID
 * Byte 1: 0x0D   Toshiba device ID
 * Byte 2: 0x03   Settings command
 * Byte 3: 0xFC   Constant
 * Byte 4: 0x01   Constant
 * Byte 5: (temp - 17) << 4  (e.g. 28°C = 0xB0; fan-only mode = 0x50)
 * Byte 6: fan_nibble << 4 | mode_nibble
 *           fan:  0x0=auto, 0x4=low, 0x6=middle, 0x8=medium, 0xA=high
 *           mode: 0x0=auto, 0x1=cool, 0x2=dry,   0x3=heat,   0x4=fan
 * Byte 7: 0x00   Constant
 * Byte 8: XOR of bytes 0–7 (checksum)
 *
 * --- 7-byte Swing Packet (56 bits) ---
 * Byte 0: 0xF2
 * Byte 1: 0x0D
 * Byte 2: 0x01   Swing command
 * Byte 3: 0xFE   Constant
 * Byte 4: 0x21 = swing vertical ON,  0x41 = swing OFF
 *           (0x41 is inferred; only swing-ON was captured)
 * Byte 5: 0x01   Constant
 * Byte 6: XOR of bytes 0–5 (checksum)
 */
class ToshibaAcClimate : public climate::Climate,
                         public remote_base::RemoteReceiverListener,
                         public Component {
 public:
  // Called by codegen to inject the transmitter reference
  void set_transmitter(remote_base::RemoteTransmitterBase *transmitter) {
    this->transmitter_ = transmitter;
  }

  // Component interface
  void setup() override;
  void dump_config() override;

  // Climate interface
  void control(const climate::ClimateCall &call) override;
  climate::ClimateTraits traits() override;

  // RemoteReceiverListener interface
  bool on_receive(remote_base::RemoteReceiveData data) override;

 protected:
  remote_base::RemoteTransmitterBase *transmitter_{nullptr};

  /// Tracks current swing state separately (swing packets are independent of
  /// settings packets; the AC unit maintains swing state between commands).
  bool swing_active_{false};

  /// Transmit the current mode/temperature/fan state as a 9-byte packet.
  void transmit_state_();

  /// Transmit a 7-byte swing control packet.
  void transmit_swing_(bool swing_on);

  /// Encode one byte MSB-first into IR timing data.
  static void encode_byte_(remote_base::RemoteTransmitData *dst, uint8_t byte);

  /// Decode one byte MSB-first from IR timing data.  Returns false on error.
  static bool decode_byte_(remote_base::RemoteReceiveData &src, uint8_t &byte_out);

  /// Compute XOR checksum over the first `len` bytes.
  static uint8_t compute_checksum_(const uint8_t *data, size_t len);

  /// Decode a full frame (9 or 7 bytes) from `src` into `bytes_out`.
  /// Returns the number of bytes decoded, or 0 on failure.
  static int decode_frame_(remote_base::RemoteReceiveData &src,
                            uint8_t *bytes_out, size_t max_bytes);

  /// Apply a successfully decoded 9-byte settings packet to the climate state.
  void apply_settings_packet_(const uint8_t *bytes);

  /// Apply a successfully decoded 7-byte swing packet to the climate state.
  void apply_swing_packet_(const uint8_t *bytes);
};

}  // namespace toshiba_ac
}  // namespace esphome
