#include "gree_ac.h"
#include "esphome/core/log.h"
#include <algorithm>

namespace esphome {
namespace gree_ac {

static const char *const TAG = "gree_ac";

// ─── IR timing constants (microseconds) ──────────────────────────────────────
static const uint32_t HEADER_MARK_US      = 9000;
static const uint32_t HEADER_SPACE_US     = 4500;
static const uint32_t BIT_MARK_US         =  660;
static const uint32_t BIT_ONE_SPACE_US    = 1625;
static const uint32_t BIT_ZERO_SPACE_US   =  537;
static const uint32_t INTER_FRAME_GAP_US  = 19800;  // gap between Frame 1 and Frame 2

// ─── Gree AC protocol constants ──────────────────────────────────────────────

// Byte 0 field positions
static const uint8_t B0_MODE_MASK  = 0x07;  // bits[2:0]
static const uint8_t B0_POWER_BIT  = 0x08;  // bit[3]
static const uint8_t B0_FAN_SHIFT  = 4;     // bits[5:4]
static const uint8_t B0_FAN_MASK   = 0x03;
static const uint8_t B0_SWING_BIT  = 0x40;  // bit[6]

// Mode values (Byte 0 bits[2:0])
static const uint8_t MODE_AUTO     = 0;
static const uint8_t MODE_COOL     = 1;
static const uint8_t MODE_DRY      = 2;
static const uint8_t MODE_FAN_ONLY = 3;
static const uint8_t MODE_HEAT     = 4;

// Fan values (Byte 0 bits[5:4])
static const uint8_t FAN_AUTO      = 0;
static const uint8_t FAN_LOW       = 1;
static const uint8_t FAN_MED       = 2;
static const uint8_t FAN_HIGH      = 3;

// Byte 1: temperature offset
static const float TEMP_MIN        = 16.0f;
static const float TEMP_MAX        = 30.0f;

// Byte 2 values
static const uint8_t B2_COOLING    = 0xE0;  // cool / dry
static const uint8_t B2_HEATING    = 0x60;  // heat / fan_only / auto
static const uint8_t B2_OFF        = 0xA0;  // power-off

// Byte 3 constant
static const uint8_t B3_CONST      = 0x50;

// Frame 2 constant bytes
static const uint8_t F2_B1         = 0x40;
static const uint8_t F2_B2         = 0x00;

// Trailing bits after Frame 1 data (3 bits: 0, 1, 0)
static const bool TRAILING_BITS[3] = {false, true, false};

// ─── Helpers ─────────────────────────────────────────────────────────────────

uint8_t GreeAcClimate::checksum_nibble_(const uint8_t *f1) {
  return ((f1[0] & 0xF) + (f1[1] & 0xF) + (f1[2] & 0xF) + (f1[3] & 0xF) + 0xE) & 0xF;
}

void GreeAcClimate::encode_byte_(remote_base::RemoteTransmitData *dst, uint8_t byte) {
  for (int i = 0; i < 8; i++) {  // LSB first
    dst->mark(BIT_MARK_US);
    if ((byte >> i) & 1)
      dst->space(BIT_ONE_SPACE_US);
    else
      dst->space(BIT_ZERO_SPACE_US);
  }
}

bool GreeAcClimate::decode_byte_(remote_base::RemoteReceiveData &src, uint8_t &byte_out) {
  byte_out = 0;
  for (int i = 0; i < 8; i++) {  // LSB first
    if (!src.expect_mark(BIT_MARK_US))
      return false;
    if (src.expect_space(BIT_ONE_SPACE_US)) {
      byte_out |= (1u << i);
    } else if (!src.expect_space(BIT_ZERO_SPACE_US)) {
      return false;
    }
  }
  return true;
}

// ─── Component lifecycle ─────────────────────────────────────────────────────

void GreeAcClimate::setup() {
  auto restore = this->restore_state_();
  if (restore.has_value()) {
    restore->apply(this);
  } else {
    this->mode               = climate::CLIMATE_MODE_OFF;
    this->target_temperature = 25.0f;
    this->fan_mode           = climate::CLIMATE_FAN_AUTO;
    this->swing_mode         = climate::CLIMATE_SWING_OFF;
  }
}

void GreeAcClimate::dump_config() {
  LOG_CLIMATE("", "Gree AC IR Climate", this);
}

// ─── Climate traits ──────────────────────────────────────────────────────────

climate::ClimateTraits GreeAcClimate::traits() {
  auto traits = climate::ClimateTraits();

  traits.set_supports_current_temperature(false);
  traits.set_supports_two_point_target_temperature(false);

  traits.set_visual_min_temperature(TEMP_MIN);
  traits.set_visual_max_temperature(TEMP_MAX);
  traits.set_visual_temperature_step(1.0f);

  traits.set_supported_modes({
      climate::CLIMATE_MODE_OFF,
      climate::CLIMATE_MODE_AUTO,
      climate::CLIMATE_MODE_COOL,
      climate::CLIMATE_MODE_HEAT,
      climate::CLIMATE_MODE_DRY,
      climate::CLIMATE_MODE_FAN_ONLY,
  });

  traits.set_supported_fan_modes({
      climate::CLIMATE_FAN_AUTO,
      climate::CLIMATE_FAN_LOW,
      climate::CLIMATE_FAN_MEDIUM,
      climate::CLIMATE_FAN_HIGH,
  });

  traits.set_supported_swing_modes({
      climate::CLIMATE_SWING_OFF,
      climate::CLIMATE_SWING_VERTICAL,
  });

  return traits;
}

// ─── Transmit ────────────────────────────────────────────────────────────────

void GreeAcClimate::transmit_state_() {
  if (this->transmitter_ == nullptr)
    return;

  const bool power_off = (this->mode == climate::CLIMATE_MODE_OFF);
  const bool swing_on  = (this->swing_mode == climate::CLIMATE_SWING_VERTICAL);

  // Encode mode
  uint8_t mode_val = MODE_COOL;
  switch (this->mode) {
    case climate::CLIMATE_MODE_AUTO:     mode_val = MODE_AUTO;     break;
    case climate::CLIMATE_MODE_COOL:     mode_val = MODE_COOL;     break;
    case climate::CLIMATE_MODE_DRY:      mode_val = MODE_DRY;      break;
    case climate::CLIMATE_MODE_FAN_ONLY: mode_val = MODE_FAN_ONLY; break;
    case climate::CLIMATE_MODE_HEAT:     mode_val = MODE_HEAT;     break;
    default:                             mode_val = MODE_COOL;     break;
  }

  // Encode fan
  uint8_t fan_val = FAN_AUTO;
  switch (this->fan_mode.value_or(climate::CLIMATE_FAN_AUTO)) {
    case climate::CLIMATE_FAN_LOW:    fan_val = FAN_LOW;  break;
    case climate::CLIMATE_FAN_MEDIUM: fan_val = FAN_MED;  break;
    case climate::CLIMATE_FAN_HIGH:   fan_val = FAN_HIGH; break;
    default:                          fan_val = FAN_AUTO; break;
  }

  // Clamp temperature
  float temp = std::max(TEMP_MIN, std::min(TEMP_MAX, this->target_temperature));
  uint8_t temp_val = static_cast<uint8_t>(temp - TEMP_MIN);

  // Byte 2: depends on mode
  uint8_t b2;
  if (power_off) {
    b2 = B2_OFF;
  } else if (mode_val == MODE_COOL || mode_val == MODE_DRY) {
    b2 = B2_COOLING;
  } else {
    b2 = B2_HEATING;
  }

  // Build Frame 1
  uint8_t f1[4];
  f1[0] = (uint8_t)(((uint8_t)(swing_on ? 1 : 0) << 6) |
                    ((fan_val & 0x3) << 4) |
                    (power_off ? 0 : B0_POWER_BIT) |
                    (mode_val & 0x7));
  f1[1] = temp_val & 0x0F;
  f1[2] = b2;
  f1[3] = B3_CONST;

  // Build Frame 2
  uint8_t f2[4];
  f2[0] = swing_on ? 0x01 : 0x00;
  f2[1] = F2_B1;
  f2[2] = F2_B2;
  f2[3] = checksum_nibble_(f1) << 4;

  auto call = this->transmitter_->transmit();
  auto *data = call.get_data();
  data->set_carrier_frequency(38000);

  // ── Frame 1 ──
  data->mark(HEADER_MARK_US);
  data->space(HEADER_SPACE_US);
  for (int i = 0; i < 4; i++)
    encode_byte_(data, f1[i]);
  // 3 trailing bits: 0, 1, 0
  for (bool bit : TRAILING_BITS) {
    data->mark(BIT_MARK_US);
    data->space(bit ? BIT_ONE_SPACE_US : BIT_ZERO_SPACE_US);
  }
  // Terminal mark + inter-frame gap
  data->mark(BIT_MARK_US);
  data->space(INTER_FRAME_GAP_US);

  // ── Frame 2 (no header) ──
  for (int i = 0; i < 4; i++)
    encode_byte_(data, f2[i]);
  data->mark(BIT_MARK_US);  // terminal mark

  call.perform();

  ESP_LOGD(TAG,
           "TX: pw=%d mode=%u fan=%u swing=%d temp=%.0f | "
           "F1=[%02X %02X %02X %02X] F2=[%02X %02X %02X %02X]",
           !power_off, mode_val, fan_val, (int)swing_on, temp,
           f1[0], f1[1], f1[2], f1[3],
           f2[0], f2[1], f2[2], f2[3]);
}

// ─── Control ─────────────────────────────────────────────────────────────────

void GreeAcClimate::control(const climate::ClimateCall &call) {
  if (call.get_mode().has_value())
    this->mode = *call.get_mode();
  if (call.get_target_temperature().has_value())
    this->target_temperature = *call.get_target_temperature();
  if (call.get_fan_mode().has_value())
    this->fan_mode = *call.get_fan_mode();
  if (call.get_swing_mode().has_value())
    this->swing_mode = *call.get_swing_mode();

  this->publish_state();
  this->transmit_state_();
}

// ─── Receive ─────────────────────────────────────────────────────────────────

bool GreeAcClimate::on_receive(remote_base::RemoteReceiveData data) {
  // Gree Frame 1 starts with the 9000µs header
  if (!data.expect_item(HEADER_MARK_US, HEADER_SPACE_US))
    return false;

  // Decode 4 data bytes (LSB first)
  uint8_t f1[4];
  for (int i = 0; i < 4; i++) {
    if (!decode_byte_(data, f1[i]))
      return false;
  }

  // Consume the 3 trailing bits (0, 1, 0) — values are fixed, just skip
  for (int i = 0; i < 3; i++) {
    if (!data.expect_mark(BIT_MARK_US))
      return false;
    // Accept either space length (we just skip the value)
    data.expect_space(BIT_ONE_SPACE_US) || data.expect_space(BIT_ZERO_SPACE_US);
  }

  // Extract fields from Frame 1
  const bool power_on = (f1[0] & B0_POWER_BIT) != 0;
  const uint8_t mode_val = f1[0] & B0_MODE_MASK;
  const uint8_t fan_val  = (f1[0] >> B0_FAN_SHIFT) & B0_FAN_MASK;
  const bool swing_on    = (f1[0] & B0_SWING_BIT) != 0;
  const float temp       = (float)(f1[1] & 0x0F) + TEMP_MIN;

  ESP_LOGD(TAG,
           "RX: pw=%d mode=%u fan=%u swing=%d temp=%.0f | "
           "F1=[%02X %02X %02X %02X]",
           (int)power_on, mode_val, fan_val, (int)swing_on, temp,
           f1[0], f1[1], f1[2], f1[3]);

  // Apply to climate state
  if (!power_on) {
    this->mode = climate::CLIMATE_MODE_OFF;
  } else {
    switch (mode_val) {
      case MODE_AUTO:     this->mode = climate::CLIMATE_MODE_AUTO;     break;
      case MODE_COOL:     this->mode = climate::CLIMATE_MODE_COOL;     break;
      case MODE_DRY:      this->mode = climate::CLIMATE_MODE_DRY;      break;
      case MODE_FAN_ONLY: this->mode = climate::CLIMATE_MODE_FAN_ONLY; break;
      case MODE_HEAT:     this->mode = climate::CLIMATE_MODE_HEAT;     break;
      default:            this->mode = climate::CLIMATE_MODE_COOL;     break;
    }
    this->target_temperature = temp;
  }

  switch (fan_val) {
    case FAN_LOW:  this->fan_mode = climate::CLIMATE_FAN_LOW;    break;
    case FAN_MED:  this->fan_mode = climate::CLIMATE_FAN_MEDIUM; break;
    case FAN_HIGH: this->fan_mode = climate::CLIMATE_FAN_HIGH;   break;
    default:       this->fan_mode = climate::CLIMATE_FAN_AUTO;   break;
  }

  this->swing_mode = swing_on ? climate::CLIMATE_SWING_VERTICAL : climate::CLIMATE_SWING_OFF;

  this->publish_state();
  return true;
}

}  // namespace gree_ac
}  // namespace esphome
