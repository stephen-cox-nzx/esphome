#include "vl53l1x.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"

/*
 * Most of the code in this integration is based on the VL53L1X library
 * by Pololu (Pololu Corporation), which in turn is based on the VL53L1X
 * API from ST.
 *
 * For more information about licensing, please view the included LICENSE.txt file
 * in the vl53l1x integration directory.
 */

namespace esphome {
namespace vl53l1x {

static const char *const TAG = "vl53l1x";

// Constants from VL53L1X API
static const uint16_t TARGET_RATE = 0x0A00;  // Target rate in 9.7 fixed point MCPS
static const uint32_t TIMING_GUARD = 4528;   // Timing guard in microseconds
static const uint16_t MODEL_ID = 0xEACC;     // Expected model ID

std::list<VL53L1XComponent *> VL53L1XComponent::vl53_sensors_;  // NOLINT
bool VL53L1XComponent::enable_pin_setup_complete_ = false;      // NOLINT

VL53L1XComponent::VL53L1XComponent() { VL53L1XComponent::vl53_sensors_.push_back(this); }

void VL53L1XComponent::setup() {
  ESP_LOGCONFIG(TAG, "Setting up VL53L1X...");

  // Handle enable pins for multiple sensors
  if (!VL53L1XComponent::enable_pin_setup_complete_) {
    for (auto *sensor : vl53_sensors_) {
      if (sensor->enable_pin_ != nullptr) {
        sensor->enable_pin_->setup();
        sensor->enable_pin_->digital_write(false);
        delay(10);
      }
    }
    VL53L1XComponent::enable_pin_setup_complete_ = true;
  }

  // Enable this sensor
  if (this->enable_pin_ != nullptr) {
    this->enable_pin_->digital_write(true);
    delay(10);
  }

  // Save the desired I2C address and use default 0x29 for setup
  uint8_t final_address = this->address_;
  this->set_i2c_address(0x29);

  // Initialize the sensor
  if (!this->init_sensor_()) {
    ESP_LOGE(TAG, "Failed to initialize VL53L1X sensor");
    this->mark_failed();
    return;
  }

  // Set the final I2C address if different
  if (final_address != 0x29) {
    if (!this->write_reg(I2C_SLAVE__DEVICE_ADDRESS, final_address & 0x7F)) {
      ESP_LOGE(TAG, "Failed to set I2C address");
      this->mark_failed();
      return;
    }
    this->set_i2c_address(final_address);
  }

  // Configure distance mode
  if (!this->set_distance_mode_(this->distance_mode_)) {
    ESP_LOGE(TAG, "Failed to set distance mode");
    this->mark_failed();
    return;
  }

  // Set measurement timing budget
  if (!this->set_measurement_timing_budget_(this->timing_budget_ms_ * 1000)) {
    ESP_LOGE(TAG, "Failed to set timing budget");
    this->mark_failed();
    return;
  }

  // Start continuous measurement
  this->start_single_shot_();
  this->measurement_started_ = true;
  this->measurement_start_time_ = millis();

  ESP_LOGCONFIG(TAG, "VL53L1X setup complete");
}

void VL53L1XComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "VL53L1X:");
  LOG_I2C_DEVICE(this);
  if (this->enable_pin_ != nullptr) {
    LOG_PIN("  Enable Pin: ", this->enable_pin_);
  }

  const char *mode_str = "UNKNOWN";
  switch (this->distance_mode_) {
    case SHORT:
      mode_str = "SHORT";
      break;
    case MEDIUM:
      mode_str = "MEDIUM";
      break;
    case LONG:
      mode_str = "LONG";
      break;
    default:
      break;
  }
  ESP_LOGCONFIG(TAG, "  Distance Mode: %s", mode_str);
  ESP_LOGCONFIG(TAG, "  Timing Budget: %u ms", this->timing_budget_ms_);

  if (this->is_failed()) {
    ESP_LOGE(TAG, "Communication with VL53L1X failed!");
  }
}

void VL53L1XComponent::loop() {
  if (!this->measurement_started_) {
    return;
  }

  // Check if data is ready
  if (!this->data_ready_()) {
    return;
  }

  // Read the range
  uint16_t range_mm = this->read_range_mm_();
  RangeStatus status = this->get_range_status_();

  // Clear interrupt
  this->write_reg(SYSTEM__INTERRUPT_CLEAR, 0x01);

  // Notify all listeners
  this->notify_listeners_(range_mm, status);

  // Start next measurement
  this->start_single_shot_();
  this->measurement_start_time_ = millis();
}

bool VL53L1XComponent::init_sensor_() {
  // Check model ID
  uint16_t model_id;
  if (!this->read_reg_16(IDENTIFICATION__MODEL_ID, &model_id)) {
    ESP_LOGE(TAG, "Failed to read model ID");
    return false;
  }

  if (model_id != MODEL_ID) {
    ESP_LOGE(TAG, "Invalid model ID: 0x%04X (expected 0x%04X)", model_id, MODEL_ID);
    return false;
  }

  // Software reset
  if (!this->write_reg(SOFT_RESET, 0x00)) {
    return false;
  }
  delayMicroseconds(100);
  if (!this->write_reg(SOFT_RESET, 0x01)) {
    return false;
  }
  delay(1);

  // Wait for boot
  uint32_t start_time = millis();
  uint8_t status;
  while (true) {
    if (!this->read_reg(FIRMWARE__SYSTEM_STATUS, &status)) {
      return false;
    }
    if ((status & 0x01) != 0) {
      break;
    }
    if (millis() - start_time > 100) {
      ESP_LOGE(TAG, "Timeout waiting for boot");
      return false;
    }
    delay(1);
  }

  // Read oscillator info
  if (!this->read_reg_16(OSC_MEASURED__FAST_OSC__FREQUENCY, &this->fast_osc_frequency_)) {
    return false;
  }
  if (!this->read_reg_16(RESULT__OSC_CALIBRATE_VAL, &this->osc_calibrate_val_)) {
    return false;
  }

  // Static init - default configuration
  // These values are from VL53L1_StaticInit() in the API
  if (!this->write_reg_16(DSS_CONFIG__TARGET_TOTAL_RATE_MCPS, TARGET_RATE))
    return false;
  if (!this->write_reg(GPIO__TIO_HV_STATUS, 0x02))
    return false;
  if (!this->write_reg(SIGMA_ESTIMATOR__EFFECTIVE_PULSE_WIDTH_NS, 8))
    return false;
  if (!this->write_reg(SIGMA_ESTIMATOR__EFFECTIVE_AMBIENT_WIDTH_NS, 16))
    return false;
  if (!this->write_reg(ALGO__CROSSTALK_COMPENSATION_VALID_HEIGHT_MM, 0x01))
    return false;
  if (!this->write_reg(ALGO__RANGE_IGNORE_VALID_HEIGHT_MM, 0xFF))
    return false;
  if (!this->write_reg(ALGO__RANGE_MIN_CLIP, 0))
    return false;
  if (!this->write_reg(ALGO__CONSISTENCY_CHECK__TOLERANCE, 2))
    return false;

  // General config
  if (!this->write_reg_16(SYSTEM__THRESH_RATE_HIGH, 0x0000))
    return false;
  if (!this->write_reg_16(SYSTEM__THRESH_RATE_LOW, 0x0000))
    return false;
  if (!this->write_reg(DSS_CONFIG__APERTURE_ATTENUATION, 0x38))
    return false;

  // Timing config
  if (!this->write_reg_16(RANGE_CONFIG__SIGMA_THRESH, 360))
    return false;
  if (!this->write_reg_16(RANGE_CONFIG__MIN_COUNT_RATE_RTN_LIMIT_MCPS, 192))
    return false;

  // Dynamic config
  if (!this->write_reg(SYSTEM__GROUPED_PARAMETER_HOLD_0, 0x01))
    return false;
  if (!this->write_reg(SYSTEM__GROUPED_PARAMETER_HOLD_1, 0x01))
    return false;
  if (!this->write_reg(SD_CONFIG__QUANTIFIER, 2))
    return false;

  // Set system to timed ranging mode
  if (!this->write_reg(SYSTEM__GROUPED_PARAMETER_HOLD, 0x00))
    return false;
  if (!this->write_reg(SYSTEM__SEED_CONFIG, 1))
    return false;

  // Configure for low power auto mode
  if (!this->write_reg(SYSTEM__SEQUENCE_CONFIG, 0x8B))
    return false;  // VHV, PHASECAL, DSS1, RANGE
  if (!this->write_reg_16(DSS_CONFIG__MANUAL_EFFECTIVE_SPADS_SELECT, 200 << 8))
    return false;
  if (!this->write_reg(DSS_CONFIG__ROI_MODE_CONTROL, 2))
    return false;

  // Set part to part offset
  uint16_t outer_offset_mm;
  if (!this->read_reg_16(MM_CONFIG__OUTER_OFFSET_MM, &outer_offset_mm)) {
    return false;
  }
  if (!this->write_reg_16(ALGO__PART_TO_PART_RANGE_OFFSET_MM, outer_offset_mm * 4)) {
    return false;
  }

  return true;
}

bool VL53L1XComponent::set_distance_mode(DistanceMode mode) {
  // Save existing timing budget
  uint32_t budget_us = this->get_measurement_timing_budget();

  switch (mode) {
    case SHORT:
      // Short range mode
      if (!this->write_reg(RANGE_CONFIG__VCSEL_PERIOD_A, 0x07))
        return false;
      if (!this->write_reg(RANGE_CONFIG__VCSEL_PERIOD_B, 0x05))
        return false;
      if (!this->write_reg(RANGE_CONFIG__VALID_PHASE_HIGH, 0x38))
        return false;
      if (!this->write_reg(SD_CONFIG__WOI_SD0, 0x07))
        return false;
      if (!this->write_reg(SD_CONFIG__WOI_SD1, 0x05))
        return false;
      if (!this->write_reg(SD_CONFIG__INITIAL_PHASE_SD0, 6))
        return false;
      if (!this->write_reg(SD_CONFIG__INITIAL_PHASE_SD1, 6))
        return false;
      break;

    case MEDIUM:
      // Medium range mode
      if (!this->write_reg(RANGE_CONFIG__VCSEL_PERIOD_A, 0x0B))
        return false;
      if (!this->write_reg(RANGE_CONFIG__VCSEL_PERIOD_B, 0x09))
        return false;
      if (!this->write_reg(RANGE_CONFIG__VALID_PHASE_HIGH, 0x78))
        return false;
      if (!this->write_reg(SD_CONFIG__WOI_SD0, 0x0B))
        return false;
      if (!this->write_reg(SD_CONFIG__WOI_SD1, 0x09))
        return false;
      if (!this->write_reg(SD_CONFIG__INITIAL_PHASE_SD0, 10))
        return false;
      if (!this->write_reg(SD_CONFIG__INITIAL_PHASE_SD1, 10))
        return false;
      break;

    case LONG:
      // Long range mode
      if (!this->write_reg(RANGE_CONFIG__VCSEL_PERIOD_A, 0x0F))
        return false;
      if (!this->write_reg(RANGE_CONFIG__VCSEL_PERIOD_B, 0x0D))
        return false;
      if (!this->write_reg(RANGE_CONFIG__VALID_PHASE_HIGH, 0xB8))
        return false;
      if (!this->write_reg(SD_CONFIG__WOI_SD0, 0x0F))
        return false;
      if (!this->write_reg(SD_CONFIG__WOI_SD1, 0x0D))
        return false;
      if (!this->write_reg(SD_CONFIG__INITIAL_PHASE_SD0, 14))
        return false;
      if (!this->write_reg(SD_CONFIG__INITIAL_PHASE_SD1, 14))
        return false;
      break;

    default:
      return false;
  }

  // Restore timing budget
  return this->set_measurement_timing_budget(budget_us);
}

bool VL53L1XComponent::set_measurement_timing_budget_(uint32_t budget_us) {
  // Timing budget must be at least 20ms
  if (budget_us < 20000) {
    return false;
  }

  // Calculate timeout values for range config
  uint32_t range_config_timeout_us = budget_us / 2 - TIMING_GUARD / 2;

  // Get macro period for VCSEL period A
  uint8_t vcsel_period_a;
  if (!this->read_reg(RANGE_CONFIG__VCSEL_PERIOD_A, &vcsel_period_a)) {
    return false;
  }
  uint32_t macro_period_us = this->calc_macro_period_(vcsel_period_a);

  // Update MM timing A timeout
  uint32_t timeout_mclks = this->timeout_microseconds_to_mclks_(1, macro_period_us);
  uint16_t encoded_timeout = this->encode_timeout_(timeout_mclks);
  if (!this->write_reg_16(MM_CONFIG__TIMEOUT_MACROP_A_HI, encoded_timeout)) {
    return false;
  }

  // Update range timing A timeout
  timeout_mclks = this->timeout_microseconds_to_mclks_(range_config_timeout_us, macro_period_us);
  encoded_timeout = this->encode_timeout_(timeout_mclks);
  if (!this->write_reg_16(RANGE_CONFIG__TIMEOUT_MACROP_A_HI, encoded_timeout)) {
    return false;
  }

  // Get macro period for VCSEL period B
  uint8_t vcsel_period_b;
  if (!this->read_reg(RANGE_CONFIG__VCSEL_PERIOD_B, &vcsel_period_b)) {
    return false;
  }
  macro_period_us = this->calc_macro_period_(vcsel_period_b);

  // Update MM timing B timeout
  timeout_mclks = this->timeout_microseconds_to_mclks_(1, macro_period_us);
  encoded_timeout = this->encode_timeout_(timeout_mclks);
  if (!this->write_reg_16(MM_CONFIG__TIMEOUT_MACROP_B_HI, encoded_timeout)) {
    return false;
  }

  // Update range timing B timeout
  timeout_mclks = this->timeout_microseconds_to_mclks_(range_config_timeout_us, macro_period_us);
  encoded_timeout = this->encode_timeout_(timeout_mclks);
  if (!this->write_reg_16(RANGE_CONFIG__TIMEOUT_MACROP_B_HI, encoded_timeout)) {
    return false;
  }

  return true;
}

uint32_t VL53L1XComponent::get_measurement_timing_budget() {
  // Get macro period for VCSEL period A
  uint8_t vcsel_period_a;
  if (!this->read_reg(RANGE_CONFIG__VCSEL_PERIOD_A, &vcsel_period_a)) {
    return 0;
  }
  uint32_t macro_period_us = this->calc_macro_period_(vcsel_period_a);

  // Get range timing A timeout
  uint16_t encoded_timeout;
  if (!this->read_reg_16(RANGE_CONFIG__TIMEOUT_MACROP_A_HI, &encoded_timeout)) {
    return 0;
  }
  uint32_t timeout_mclks = this->decode_timeout_(encoded_timeout);
  uint32_t range_config_timeout_us = this->timeout_mclks_to_microseconds_(timeout_mclks, macro_period_us);

  return 2 * range_config_timeout_us + TIMING_GUARD;
}

void VL53L1XComponent::start_single_shot_() {
  this->write_reg(SYSTEM__INTERRUPT_CLEAR, 0x01);
  this->write_reg(SYSTEM__MODE_START, 0x10);  // Single shot mode
}

bool VL53L1XComponent::data_ready_() {
  uint8_t gpio_status;
  if (!this->read_reg(GPIO__TIO_HV_STATUS, &gpio_status)) {
    return false;
  }
  return (gpio_status & 0x01) != 0;
}

uint16_t VL53L1XComponent::read_range_mm() {
  uint16_t range_mm;
  if (!this->read_reg_16(RESULT__FINAL__CROSSTALK_CORRECTED_RANGE_MM_SD0, &range_mm)) {
    return 0;
  }
  return range_mm;
}

RangeStatus VL53L1XComponent::get_range_status_() {
  uint8_t status;
  if (!this->read_reg(RESULT__RANGE_STATUS, &status)) {
    return NONE;
  }
  // Extract range status from bits 4:0
  return static_cast<RangeStatus>(status & 0x1F);
}

// Helper methods for register access
bool VL53L1XComponent::write_reg(uint16_t reg, uint8_t value) {
  uint8_t data[3];
  data[0] = (reg >> 8) & 0xFF;  // Register high byte
  data[1] = reg & 0xFF;         // Register low byte
  data[2] = value;
  return this->write_bytes_raw(data, 3);
}

bool VL53L1XComponent::write_reg_16(uint16_t reg, uint16_t value) {
  uint8_t data[4];
  data[0] = (reg >> 8) & 0xFF;    // Register high byte
  data[1] = reg & 0xFF;           // Register low byte
  data[2] = (value >> 8) & 0xFF;  // Value high byte
  data[3] = value & 0xFF;         // Value low byte
  return this->write_bytes_raw(data, 4);
}

bool VL53L1XComponent::write_reg_32(uint16_t reg, uint32_t value) {
  uint8_t data[6];
  data[0] = (reg >> 8) & 0xFF;     // Register high byte
  data[1] = reg & 0xFF;            // Register low byte
  data[2] = (value >> 24) & 0xFF;  // Value highest byte
  data[3] = (value >> 16) & 0xFF;
  data[4] = (value >> 8) & 0xFF;
  data[5] = value & 0xFF;  // Value lowest byte
  return this->write_bytes_raw(data, 6);
}

bool VL53L1XComponent::read_reg(uint16_t reg, uint8_t *value) {
  uint8_t reg_addr[2];
  reg_addr[0] = (reg >> 8) & 0xFF;
  reg_addr[1] = reg & 0xFF;
  if (!this->write_bytes_raw(reg_addr, 2)) {
    return false;
  }
  return this->read_bytes_raw(value, 1);
}

bool VL53L1XComponent::read_reg_16(uint16_t reg, uint16_t *value) {
  uint8_t reg_addr[2];
  reg_addr[0] = (reg >> 8) & 0xFF;
  reg_addr[1] = reg & 0xFF;
  if (!this->write_bytes_raw(reg_addr, 2)) {
    return false;
  }
  uint8_t data[2];
  if (!this->read_bytes_raw(data, 2)) {
    return false;
  }
  *value = (data[0] << 8) | data[1];
  return true;
}

bool VL53L1XComponent::read_reg_32(uint16_t reg, uint32_t *value) {
  uint8_t reg_addr[2];
  reg_addr[0] = (reg >> 8) & 0xFF;
  reg_addr[1] = reg & 0xFF;
  if (!this->write_bytes_raw(reg_addr, 2)) {
    return false;
  }
  uint8_t data[4];
  if (!this->read_bytes_raw(data, 4)) {
    return false;
  }
  *value = ((uint32_t) data[0] << 24) | ((uint32_t) data[1] << 16) | ((uint32_t) data[2] << 8) | data[3];
  return true;
}

// Timing calculation helpers
uint32_t VL53L1XComponent::calc_macro_period_(uint8_t vcsel_period) {
  // Macro period in microseconds = 2304 * (vcsel_period * 1.655) / 1000
  // Simplified: (2304 * vcsel_period * 1655 + 500) / 1000000
  uint32_t pll_period_us = ((uint32_t) 0x01 << 30) / this->fast_osc_frequency_;
  uint8_t vcsel_period_pclks = (vcsel_period + 1) << 1;
  uint32_t macro_period_us = (uint32_t) 2304 * pll_period_us;
  macro_period_us >>= 6;
  macro_period_us *= vcsel_period_pclks;
  macro_period_us >>= 6;
  return macro_period_us;
}

uint32_t VL53L1XComponent::timeout_mclks_to_microseconds_(uint32_t timeout_mclks, uint32_t macro_period_us) {
  return ((uint64_t) timeout_mclks * macro_period_us + 0x800) >> 12;
}

uint32_t VL53L1XComponent::timeout_microseconds_to_mclks_(uint32_t timeout_us, uint32_t macro_period_us) {
  return (((uint32_t) timeout_us << 12) + (macro_period_us >> 1)) / macro_period_us;
}

uint16_t VL53L1XComponent::encode_timeout_(uint32_t timeout_mclks) {
  // Encode timeout in format (LSByte * 2^MSByte) + 1
  uint32_t ls_byte = 0;
  uint16_t ms_byte = 0;

  if (timeout_mclks > 0) {
    ls_byte = timeout_mclks - 1;

    while ((ls_byte & 0xFFFFFF00) > 0) {
      ls_byte >>= 1;
      ms_byte++;
    }
  }

  return (ms_byte << 8) | (ls_byte & 0xFF);
}

uint32_t VL53L1XComponent::decode_timeout_(uint16_t reg_val) {
  // Decode timeout from format (LSByte * 2^MSByte) + 1
  uint8_t ms_byte = (reg_val >> 8) & 0xFF;
  uint8_t ls_byte = reg_val & 0xFF;
  return (((uint32_t) ls_byte) << ms_byte) + 1;
}

void VL53L1XComponent::notify_listeners_(uint16_t distance_mm, RangeStatus status) {
  for (auto *listener : this->listeners_) {
    listener->on_distance(distance_mm, status);
  }
}

}  // namespace vl53l1x
}  // namespace esphome
