#include "toshiba_ac.h"
#include "esphome/core/log.h"
#include <algorithm>
#include <cstring>

namespace esphome {
namespace toshiba_ac {

static const char *const TAG = "toshiba_ac";

// ─── IR timing constants (microseconds) ──────────────────────────────────────
static const uint32_t HEADER_MARK_US  = 4500;
static const uint32_t HEADER_SPACE_US = 4500;
static const uint32_t BIT_MARK_US     =  560;
static const uint32_t BIT_ONE_SPACE_US = 1600;
static const uint32_t BIT_ZERO_SPACE_US =  560;
static const uint32_t REPEAT_GAP_US   = 5500;  // inter-frame gap

// ─── Toshiba AC protocol constants ───────────────────────────────────────────
static const uint8_t TOSHIBA_ID1    = 0xF2;
static const uint8_t TOSHIBA_ID2    = 0x0D;

// Byte 2 values that identify the packet type
static const uint8_t CMD_SETTINGS   = 0x03;  // 9-byte settings packet
static const uint8_t CMD_SWING      = 0x01;  // 7-byte swing packet

// Fixed bytes in 9-byte settings packet
static const uint8_t SETTINGS_B3    = 0xFC;
static const uint8_t SETTINGS_B4    = 0x01;

// Fixed bytes in 7-byte swing packet
// Verified: ON  = F2 0D 01 FE 21 01 20
//           OFF = F2 0D 01 FE 21 02 23
static const uint8_t SWING_B3         = 0xFE;
static const uint8_t SWING_B4         = 0x21;  // constant in both ON and OFF
static const uint8_t SWING_ON_SPEED   = 0x01;  // byte 5 when swing is ON
static const uint8_t SWING_OFF_SPEED  = 0x02;  // byte 5 when swing is OFF

// Mode nibbles — lower nibble of byte 6 in 9-byte packet
// Verified from captures: cool=0x1, dry=0x2, fan=0x4, off=0x7
// Heat=0x3 and auto=0x0 are inferred from protocol structure
static const uint8_t MODE_AUTO  = 0x0;
static const uint8_t MODE_COOL  = 0x1;
static const uint8_t MODE_DRY   = 0x2;
static const uint8_t MODE_HEAT  = 0x3;  // inferred
static const uint8_t MODE_FAN   = 0x4;
static const uint8_t MODE_OFF   = 0x7;  // verified from captures

// Fan speed nibbles — upper nibble of byte 6 (raw value, not pre-shifted)
// Verified from captures: cool mode fan auto/1/2/3/4
static const uint8_t FAN_AUTO   = 0x0;
static const uint8_t FAN_SPEED1 = 0x4;  // low
static const uint8_t FAN_SPEED2 = 0x6;  // middle
static const uint8_t FAN_SPEED3 = 0x8;  // medium
static const uint8_t FAN_SPEED4 = 0xA;  // high

// Temperature encoding:  byte5 upper nibble = (temp_celsius - 17)
// Supported range: 17–30°C
static const float TEMP_MIN     = 17.0f;
static const float TEMP_MAX     = 30.0f;
static const float FAN_MODE_TEMP = 22.0f;  // default temp sent in fan-only mode

// ─── Helpers ─────────────────────────────────────────────────────────────────

uint8_t ToshibaAcClimate::compute_checksum_(const uint8_t *data, size_t len) {
  uint8_t cs = 0;
  for (size_t i = 0; i < len; i++)
    cs ^= data[i];
  return cs;
}

void ToshibaAcClimate::encode_byte_(remote_base::RemoteTransmitData *dst, uint8_t byte) {
  for (int i = 7; i >= 0; i--) {  // MSB first
    dst->mark(BIT_MARK_US);
    if ((byte >> i) & 1)
      dst->space(BIT_ONE_SPACE_US);
    else
      dst->space(BIT_ZERO_SPACE_US);
  }
}

bool ToshibaAcClimate::decode_byte_(remote_base::RemoteReceiveData &src, uint8_t &byte_out) {
  byte_out = 0;
  for (int i = 7; i >= 0; i--) {  // MSB first
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

/**
 * Decode a single frame starting at the current position in `src`.
 * Reads header, then up to max_bytes bytes, then the terminal mark.
 * Returns the number of bytes decoded (7 or 9), or 0 on failure.
 * Does NOT consume the repeat-gap space (caller handles it).
 */
int ToshibaAcClimate::decode_frame_(remote_base::RemoteReceiveData &src,
                                     uint8_t *bytes_out, size_t max_bytes) {
  if (!src.expect_item(HEADER_MARK_US, HEADER_SPACE_US))
    return 0;

  // Decode the first 3 bytes to determine packet type and length
  for (int i = 0; i < 3; i++) {
    if (!decode_byte_(src, bytes_out[i]))
      return 0;
  }

  // Verify Toshiba device ID
  if (bytes_out[0] != TOSHIBA_ID1 || bytes_out[1] != TOSHIBA_ID2)
    return 0;

  // Determine total packet length
  int packet_len;
  if (bytes_out[2] == CMD_SETTINGS)
    packet_len = 9;
  else if (bytes_out[2] == CMD_SWING)
    packet_len = 7;
  else
    return 0;  // unknown command

  if ((size_t) packet_len > max_bytes)
    return 0;

  // Decode remaining bytes
  for (int i = 3; i < packet_len; i++) {
    if (!decode_byte_(src, bytes_out[i]))
      return 0;
  }

  // Verify checksum
  if (compute_checksum_(bytes_out, packet_len - 1) != bytes_out[packet_len - 1])
    return 0;

  // Consume terminal burst mark
  src.expect_mark(BIT_MARK_US);

  return packet_len;
}

// ─── Component lifecycle ─────────────────────────────────────────────────────

void ToshibaAcClimate::setup() {
  // Restore last known state from flash
  auto restore = this->restore_state_();
  if (restore.has_value()) {
    restore->apply(this);
    // Restore swing mode from the ClimateState
    this->swing_active_ = (this->swing_mode == climate::CLIMATE_SWING_VERTICAL);
  } else {
    // Sensible defaults
    this->mode            = climate::CLIMATE_MODE_OFF;
    this->target_temperature = 25.0f;
    this->fan_mode        = climate::CLIMATE_FAN_AUTO;
    this->swing_mode      = climate::CLIMATE_SWING_OFF;
    this->swing_active_   = false;
  }
}

void ToshibaAcClimate::dump_config() {
  LOG_CLIMATE("", "Toshiba AC IR Climate", this);
}

// ─── Climate traits ──────────────────────────────────────────────────────────

climate::ClimateTraits ToshibaAcClimate::traits() {
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
      climate::CLIMATE_FAN_MIDDLE,
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

void ToshibaAcClimate::transmit_state_() {
  if (this->transmitter_ == nullptr)
    return;

  uint8_t bytes[9];
  bytes[0] = TOSHIBA_ID1;
  bytes[1] = TOSHIBA_ID2;
  bytes[2] = CMD_SETTINGS;
  bytes[3] = SETTINGS_B3;
  bytes[4] = SETTINGS_B4;

  // Byte 5: temperature nibble (OFF mode preserves last temperature)
  uint8_t temp_nibble;
  if (this->mode == climate::CLIMATE_MODE_FAN_ONLY) {
    temp_nibble = static_cast<uint8_t>(FAN_MODE_TEMP - TEMP_MIN);
  } else {
    float t = std::max(TEMP_MIN, std::min(TEMP_MAX, this->target_temperature));
    temp_nibble = static_cast<uint8_t>(t - TEMP_MIN);
  }
  bytes[5] = (temp_nibble << 4) | 0x00;

  // Byte 6: fan nibble | mode nibble
  uint8_t fan_nibble = FAN_AUTO;
  switch (this->fan_mode.value_or(climate::CLIMATE_FAN_AUTO)) {
    case climate::CLIMATE_FAN_LOW:    fan_nibble = FAN_SPEED1; break;
    case climate::CLIMATE_FAN_MIDDLE: fan_nibble = FAN_SPEED2; break;
    case climate::CLIMATE_FAN_MEDIUM: fan_nibble = FAN_SPEED3; break;
    case climate::CLIMATE_FAN_HIGH:   fan_nibble = FAN_SPEED4; break;
    default:                          fan_nibble = FAN_AUTO;   break;
  }

  uint8_t mode_nibble = MODE_COOL;
  switch (this->mode) {
    case climate::CLIMATE_MODE_OFF:      mode_nibble = MODE_OFF;  break;
    case climate::CLIMATE_MODE_COOL:     mode_nibble = MODE_COOL; break;
    case climate::CLIMATE_MODE_HEAT:     mode_nibble = MODE_HEAT; break;
    case climate::CLIMATE_MODE_DRY:
      mode_nibble = MODE_DRY;
      fan_nibble  = FAN_AUTO;  // dry mode always uses auto fan (per captures)
      break;
    case climate::CLIMATE_MODE_FAN_ONLY: mode_nibble = MODE_FAN;  break;
    case climate::CLIMATE_MODE_AUTO:     mode_nibble = MODE_AUTO; break;
    default:                             mode_nibble = MODE_COOL; break;
  }

  bytes[6] = (fan_nibble << 4) | mode_nibble;
  bytes[7] = 0x00;
  bytes[8] = compute_checksum_(bytes, 8);

  auto call = this->transmitter_->transmit();
  auto *data = call.get_data();
  data->set_carrier_frequency(38000);

  // Frame 1
  data->item(HEADER_MARK_US, HEADER_SPACE_US);
  for (int i = 0; i < 9; i++)
    encode_byte_(data, bytes[i]);
  data->item(BIT_MARK_US, REPEAT_GAP_US);

  // Frame 2 (repeat)
  data->item(HEADER_MARK_US, HEADER_SPACE_US);
  for (int i = 0; i < 9; i++)
    encode_byte_(data, bytes[i]);
  data->mark(BIT_MARK_US);

  call.perform();

  ESP_LOGD(TAG, "TX settings: mode=%u fan=0x%X temp_nib=0x%X | %02X %02X %02X %02X %02X %02X %02X %02X %02X",
           mode_nibble, fan_nibble, temp_nibble,
           bytes[0], bytes[1], bytes[2], bytes[3], bytes[4],
           bytes[5], bytes[6], bytes[7], bytes[8]);
}

void ToshibaAcClimate::transmit_swing_(bool swing_on) {
  if (this->transmitter_ == nullptr)
    return;

  uint8_t bytes[7];
  bytes[0] = TOSHIBA_ID1;
  bytes[1] = TOSHIBA_ID2;
  bytes[2] = CMD_SWING;
  bytes[3] = SWING_B3;
  bytes[4] = SWING_B4;
  bytes[5] = swing_on ? SWING_ON_SPEED : SWING_OFF_SPEED;
  bytes[6] = compute_checksum_(bytes, 6);

  auto call = this->transmitter_->transmit();
  auto *data = call.get_data();
  data->set_carrier_frequency(38000);

  // Frame 1
  data->item(HEADER_MARK_US, HEADER_SPACE_US);
  for (int i = 0; i < 7; i++)
    encode_byte_(data, bytes[i]);
  data->item(BIT_MARK_US, REPEAT_GAP_US);

  // Frame 2 (repeat)
  data->item(HEADER_MARK_US, HEADER_SPACE_US);
  for (int i = 0; i < 7; i++)
    encode_byte_(data, bytes[i]);
  data->mark(BIT_MARK_US);

  call.perform();

  ESP_LOGD(TAG, "TX swing: %s | %02X %02X %02X %02X %02X %02X %02X",
           swing_on ? "ON" : "OFF",
           bytes[0], bytes[1], bytes[2], bytes[3], bytes[4], bytes[5], bytes[6]);
}

// ─── Receive ─────────────────────────────────────────────────────────────────

bool ToshibaAcClimate::on_receive(remote_base::RemoteReceiveData data) {
  uint8_t frame1[9] = {};
  uint8_t frame2[9] = {};

  // Decode first frame
  int len1 = decode_frame_(data, frame1, sizeof(frame1));
  if (len1 == 0)
    return false;

  // Consume the repeat-gap space between the two frames
  if (!data.expect_space(REPEAT_GAP_US))
    return false;

  // Decode second frame (must be identical)
  int len2 = decode_frame_(data, frame2, sizeof(frame2));
  if (len2 != len1)
    return false;

  if (memcmp(frame1, frame2, len1) != 0) {
    ESP_LOGW(TAG, "RX: frame mismatch, ignoring");
    return false;
  }

  // Apply decoded state
  if (len1 == 9 && frame1[2] == CMD_SETTINGS) {
    apply_settings_packet_(frame1);
    return true;
  }
  if (len1 == 7 && frame1[2] == CMD_SWING) {
    apply_swing_packet_(frame1);
    return true;
  }

  return false;
}

void ToshibaAcClimate::apply_settings_packet_(const uint8_t *bytes) {
  uint8_t temp_nibble = (bytes[5] >> 4) & 0x0F;
  uint8_t fan_nibble  = (bytes[6] >> 4) & 0x0F;
  uint8_t mode_nibble =  bytes[6]       & 0x0F;

  // Decode mode
  switch (mode_nibble) {
    case MODE_COOL:
      this->mode = climate::CLIMATE_MODE_COOL;
      this->target_temperature = TEMP_MIN + temp_nibble;
      break;
    case MODE_HEAT:
      this->mode = climate::CLIMATE_MODE_HEAT;
      this->target_temperature = TEMP_MIN + temp_nibble;
      break;
    case MODE_DRY:
      this->mode = climate::CLIMATE_MODE_DRY;
      this->target_temperature = TEMP_MIN + temp_nibble;
      break;
    case MODE_FAN:
      this->mode = climate::CLIMATE_MODE_FAN_ONLY;
      this->target_temperature = FAN_MODE_TEMP;
      break;
    case MODE_AUTO:
      this->mode = climate::CLIMATE_MODE_AUTO;
      this->target_temperature = TEMP_MIN + temp_nibble;
      break;
    case MODE_OFF:
      this->mode = climate::CLIMATE_MODE_OFF;
      break;
    default:
      ESP_LOGW(TAG, "RX: unknown mode nibble 0x%X", mode_nibble);
      return;
  }

  // Decode fan speed
  switch (fan_nibble) {
    case FAN_AUTO:   this->fan_mode = climate::CLIMATE_FAN_AUTO;   break;
    case FAN_SPEED1: this->fan_mode = climate::CLIMATE_FAN_LOW;    break;
    case FAN_SPEED2: this->fan_mode = climate::CLIMATE_FAN_MIDDLE; break;
    case FAN_SPEED3: this->fan_mode = climate::CLIMATE_FAN_MEDIUM; break;
    case FAN_SPEED4: this->fan_mode = climate::CLIMATE_FAN_HIGH;   break;
    default:
      ESP_LOGW(TAG, "RX: unknown fan nibble 0x%X, defaulting to AUTO", fan_nibble);
      this->fan_mode = climate::CLIMATE_FAN_AUTO;
      break;
  }

  ESP_LOGI(TAG, "RX settings: mode=%u fan=0x%X temp=%.0f°C",
           mode_nibble, fan_nibble, this->target_temperature);

  this->publish_state();
}

void ToshibaAcClimate::apply_swing_packet_(const uint8_t *bytes) {
  this->swing_active_ = (bytes[5] == SWING_ON_SPEED);
  this->swing_mode = this->swing_active_
      ? climate::CLIMATE_SWING_VERTICAL
      : climate::CLIMATE_SWING_OFF;

  ESP_LOGI(TAG, "RX swing: %s", this->swing_active_ ? "ON" : "OFF");

  this->publish_state();
}

// ─── Control (from Home Assistant) ───────────────────────────────────────────

void ToshibaAcClimate::control(const climate::ClimateCall &call) {
  bool send_settings = false;
  bool send_swing    = false;

  if (call.get_mode().has_value()) {
    this->mode = *call.get_mode();
    send_settings = true;
  }

  if (call.get_target_temperature().has_value()) {
    this->target_temperature = *call.get_target_temperature();
    send_settings = true;
  }

  if (call.get_fan_mode().has_value()) {
    this->fan_mode = *call.get_fan_mode();
    send_settings = true;
  }

  if (call.get_swing_mode().has_value()) {
    bool want_swing = (*call.get_swing_mode() == climate::CLIMATE_SWING_VERTICAL);
    if (want_swing != this->swing_active_) {
      this->swing_active_ = want_swing;
      this->swing_mode    = *call.get_swing_mode();
      send_swing = true;
    }
  }

  // Transmit IR commands (OFF is a real packet with mode nibble 0x7)
  if (send_settings)
    transmit_state_();

  if (send_swing)
    transmit_swing_(this->swing_active_);

  this->publish_state();
}

}  // namespace toshiba_ac
}  // namespace esphome
