#pragma once

#include "esphome/core/component.h"
#include "esphome/components/climate/climate.h"
#include "esphome/components/remote_base/remote_base.h"

namespace esphome {
namespace gree_ac {

/**
 * Gree AC IR Climate Component
 *
 * Implements the Gree AC 4-byte IR protocol reverse-engineered from
 * raw remote captures. Sends and receives IR signals to control the AC.
 *
 * === Protocol (LSB-first, pulse-distance encoding) ===
 *
 * Carrier:  38 kHz
 * Header:   9000µs mark + 4500µs space
 * Bit mark: ~660µs
 * Bit '1':  660µs mark + 1625µs space
 * Bit '0':  660µs mark +  537µs space
 * Terminal burst: 660µs mark
 * Inter-frame gap: ~19800µs space
 *
 * Each command consists of two frames:
 *
 * --- Frame 1 (32 data bits + 3 trailing bits) ---
 * Byte 0:  bit[6]=swing, bits[5:4]=fan, bit[3]=power, bits[2:0]=mode
 *            mode:  0=auto, 1=cool, 2=dry, 3=fan_only, 4=heat
 *            power: 0=OFF, 1=ON
 *            fan:   0=auto, 1=low, 2=med, 3=high
 *            swing: 0=off,  1=on
 * Byte 1:  bits[3:0] = (temperature_celsius - 16)   [range: 16–30°C]
 * Byte 2:  0xE0 for cool/dry, 0x60 for heat/fan/auto, 0xA0 for power-off
 * Byte 3:  0x50 (constant)
 * Trailing bits: 0, 1, 0  (3 fixed bits after byte 3)
 *
 * --- Frame 2 (32 bits, no header) ---
 * Byte 0:  0x01 if swing=on, 0x00 if swing=off   (mirrors Frame 1 swing)
 * Byte 1:  0x40 (constant)
 * Byte 2:  0x00 (constant)
 * Byte 3:  checksum_nibble << 4
 *            checksum_nibble = ((b0 & 0xF) + (b1 & 0xF) + (b2 & 0xF) + (b3 & 0xF) + 0xE) & 0xF
 *            where b0..b3 are Frame 1 bytes
 */
class GreeAcClimate : public climate::Climate,
                      public remote_base::RemoteReceiverListener,
                      public Component {
 public:
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

  /// Transmit the current state as a Gree two-frame IR command.
  void transmit_state_();

  /// Encode one byte LSB-first into the transmit buffer.
  void encode_byte_(remote_base::RemoteTransmitData *dst, uint8_t byte);

  /// Decode one byte LSB-first from the receive buffer.
  bool decode_byte_(remote_base::RemoteReceiveData &src, uint8_t &byte_out);

  /// Compute Frame 2 checksum nibble from Frame 1 bytes.
  static uint8_t checksum_nibble_(const uint8_t *f1);
};

}  // namespace gree_ac
}  // namespace esphome
