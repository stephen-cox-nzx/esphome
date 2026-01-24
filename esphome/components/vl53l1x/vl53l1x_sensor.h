#pragma once

#include "esphome/core/component.h"
#include "esphome/core/hal.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/i2c/i2c.h"

namespace esphome {
namespace vl53l1x {

// Distance modes for VL53L1X
enum DistanceMode {
  SHORT = 1,
  MEDIUM = 2,
  LONG = 3,
  UNKNOWN = 0,
};

// Register addresses from VL53L1X API
enum VL53L1XRegister : uint16_t {
  SOFT_RESET = 0x0000,
  I2C_SLAVE__DEVICE_ADDRESS = 0x0001,
  VHV_CONFIG__TIMEOUT_MACROP_LOOP_BOUND = 0x0008,
  VHV_CONFIG__INIT = 0x000B,
  PHASECAL_CONFIG__OVERRIDE = 0x004D,
  DSS_CONFIG__TARGET_TOTAL_RATE_MCPS = 0x0024,
  GPIO__TIO_HV_STATUS = 0x0031,
  SIGMA_ESTIMATOR__EFFECTIVE_PULSE_WIDTH_NS = 0x0036,
  SIGMA_ESTIMATOR__EFFECTIVE_AMBIENT_WIDTH_NS = 0x0037,
  ALGO__CROSSTALK_COMPENSATION_VALID_HEIGHT_MM = 0x0039,
  ALGO__RANGE_IGNORE_VALID_HEIGHT_MM = 0x003E,
  ALGO__RANGE_IGNORE_THRESHOLD_MCPS = 0x003C,
  ALGO__RANGE_MIN_CLIP = 0x003F,
  ALGO__CONSISTENCY_CHECK__TOLERANCE = 0x0040,
  SYSTEM__THRESH_RATE_HIGH = 0x0050,
  SYSTEM__THRESH_RATE_LOW = 0x0052,
  DSS_CONFIG__APERTURE_ATTENUATION = 0x0057,
  RANGE_CONFIG__SIGMA_THRESH = 0x0064,
  RANGE_CONFIG__MIN_COUNT_RATE_RTN_LIMIT_MCPS = 0x0066,
  SYSTEM__INTERMEASUREMENT_PERIOD = 0x006C,
  SYSTEM__FRACTIONAL_ENABLE = 0x0070,
  SYSTEM__GROUPED_PARAMETER_HOLD_0 = 0x0071,
  SYSTEM__GROUPED_PARAMETER_HOLD_1 = 0x0082,
  SYSTEM__GROUPED_PARAMETER_HOLD = 0x0082,
  SYSTEM__SEED_CONFIG = 0x0077,
  SD_CONFIG__QUANTIFIER = 0x007F,
  SYSTEM__SEQUENCE_CONFIG = 0x0081,
  DSS_CONFIG__MANUAL_EFFECTIVE_SPADS_SELECT = 0x0054,
  DSS_CONFIG__ROI_MODE_CONTROL = 0x004F,
  RANGE_CONFIG__VCSEL_PERIOD_A = 0x0060,
  RANGE_CONFIG__VCSEL_PERIOD_B = 0x0063,
  RANGE_CONFIG__VALID_PHASE_HIGH = 0x0069,
  SD_CONFIG__WOI_SD0 = 0x0078,
  SD_CONFIG__WOI_SD1 = 0x0079,
  SD_CONFIG__INITIAL_PHASE_SD0 = 0x007A,
  SD_CONFIG__INITIAL_PHASE_SD1 = 0x007B,
  RANGE_CONFIG__TIMEOUT_MACROP_A_HI = 0x005E,
  RANGE_CONFIG__TIMEOUT_MACROP_A_LO = 0x005F,
  RANGE_CONFIG__TIMEOUT_MACROP_B_HI = 0x0061,
  RANGE_CONFIG__TIMEOUT_MACROP_B_LO = 0x0062,
  MM_CONFIG__TIMEOUT_MACROP_A_HI = 0x005A,
  MM_CONFIG__TIMEOUT_MACROP_A_LO = 0x005B,
  MM_CONFIG__TIMEOUT_MACROP_B_HI = 0x005C,
  MM_CONFIG__TIMEOUT_MACROP_B_LO = 0x005D,
  MM_CONFIG__OUTER_OFFSET_MM = 0x0022,
  ALGO__PART_TO_PART_RANGE_OFFSET_MM = 0x001E,
  SYSTEM__INTERRUPT_CLEAR = 0x0086,
  SYSTEM__MODE_START = 0x0087,
  FIRMWARE__SYSTEM_STATUS = 0x00E5,
  IDENTIFICATION__MODEL_ID = 0x010F,
  OSC_MEASURED__FAST_OSC__FREQUENCY = 0x0006,
  RESULT__OSC_CALIBRATE_VAL = 0x00DE,
  PAD_I2C_HV__EXTSUP_CONFIG = 0x002E,
  GPIO__TIO_HV_STATUS_REG = 0x0031,
  GPIO_HV_MUX__CTRL = 0x0030,
  RESULT__RANGE_STATUS = 0x0089,
  RESULT__FINAL__CROSSTALK_CORRECTED_RANGE_MM_SD0 = 0x0096,
  RESULT__PEAK_SIGNAL_COUNT_RATE_CROSSTALK_CORRECTED_MCPS_SD0 = 0x0098,
  DSS_CALC__ROI_CTRL = 0x00B5,
  DSS_CALC__SPARE_1 = 0x00B6,
  DSS_CALC__SPARE_2 = 0x00B7,
  DSS_CALC__SPARE_3 = 0x00B8,
  DSS_CALC__SPARE_4 = 0x00B9,
  DSS_CALC__SPARE_5 = 0x00BA,
  DSS_CALC__SPARE_6 = 0x00BB,
  DSS_CALC__SPARE_7 = 0x00BC,
  DSS_CALC__USER_ROI_SPAD_EN_0 = 0x00BD,
  DSS_CALC__USER_ROI_SPAD_EN_1 = 0x00BE,
  DSS_CALC__USER_ROI_SPAD_EN_2 = 0x00BF,
  DSS_CALC__USER_ROI_SPAD_EN_3 = 0x00C0,
  DSS_CALC__USER_ROI_SPAD_EN_4 = 0x00C1,
  DSS_CALC__USER_ROI_SPAD_EN_5 = 0x00C2,
  DSS_CALC__USER_ROI_SPAD_EN_6 = 0x00C3,
  DSS_CALC__USER_ROI_SPAD_EN_7 = 0x00C4,
  DSS_CALC__USER_ROI_SPAD_EN_8 = 0x00C5,
  DSS_CALC__USER_ROI_SPAD_EN_9 = 0x00C6,
  DSS_CALC__USER_ROI_SPAD_EN_10 = 0x00C7,
  DSS_CALC__USER_ROI_SPAD_EN_11 = 0x00C8,
  DSS_CALC__USER_ROI_SPAD_EN_12 = 0x00C9,
  DSS_CALC__USER_ROI_SPAD_EN_13 = 0x00CA,
  DSS_CALC__USER_ROI_SPAD_EN_14 = 0x00CB,
  DSS_CALC__USER_ROI_SPAD_EN_15 = 0x00CC,
  DSS_CALC__USER_ROI_SPAD_EN_16 = 0x00CD,
  DSS_CALC__USER_ROI_SPAD_EN_17 = 0x00CE,
  DSS_CALC__USER_ROI_SPAD_EN_18 = 0x00CF,
  DSS_CALC__USER_ROI_SPAD_EN_19 = 0x00D0,
  DSS_CALC__USER_ROI_SPAD_EN_20 = 0x00D1,
  DSS_CALC__USER_ROI_SPAD_EN_21 = 0x00D2,
  DSS_CALC__USER_ROI_SPAD_EN_22 = 0x00D3,
  DSS_CALC__USER_ROI_SPAD_EN_23 = 0x00D4,
  DSS_CALC__USER_ROI_SPAD_EN_24 = 0x00D5,
  DSS_CALC__USER_ROI_SPAD_EN_25 = 0x00D6,
  DSS_CALC__USER_ROI_SPAD_EN_26 = 0x00D7,
  DSS_CALC__USER_ROI_SPAD_EN_27 = 0x00D8,
  DSS_CALC__USER_ROI_SPAD_EN_28 = 0x00D9,
  DSS_CALC__USER_ROI_SPAD_EN_29 = 0x00DA,
  VHV_RESULT__COLDBOOT_STATUS = 0x00DB,  // Also used as DSS_CALC__USER_ROI_SPAD_EN_30
  DSS_CALC__USER_ROI_SPAD_EN_31 = 0x00DC,
};

// Range status values
enum RangeStatus {
  RANGE_VALID = 0,
  SIGMA_FAIL = 1,
  SIGNAL_FAIL = 2,
  RANGE_VALID_MIN_RANGE_CLIPPED = 3,
  OUT_OF_BOUNDS_FAIL = 4,
  HARDWARE_FAIL = 5,
  RANGE_VALID_NO_WRAP_CHECK_FAIL = 6,
  WRAP_TARGET_FAIL = 7,
  XTALK_SIGNAL_FAIL = 9,
  SYNCHRONIZATION_INT = 10,
  MIN_RANGE_FAIL = 13,
  NONE = 255,
};

class VL53L1XSensor : public sensor::Sensor, public PollingComponent, public i2c::I2CDevice {
 public:
  VL53L1XSensor();

  void setup() override;
  void dump_config() override;
  void update() override;
  void loop() override;

  void set_distance_mode(DistanceMode mode) { this->distance_mode_ = mode; }
  void set_timing_budget(uint32_t timing_budget_ms) { this->timing_budget_ms_ = timing_budget_ms; }
  void set_timeout(uint32_t timeout_ms) { this->timeout_ms_ = timeout_ms; }
  void set_enable_pin(GPIOPin *enable) { this->enable_pin_ = enable; }

 protected:
  bool init_sensor_();
  bool set_distance_mode_(DistanceMode mode);
  bool set_measurement_timing_budget_(uint32_t budget_us);
  uint32_t get_measurement_timing_budget_();
  void start_single_shot_();
  bool data_ready_();
  uint16_t read_range_mm_();
  RangeStatus get_range_status_();

  // Helper methods for register access
  bool write_reg_(uint16_t reg, uint8_t value);
  bool write_reg_16_(uint16_t reg, uint16_t value);
  bool write_reg_32_(uint16_t reg, uint32_t value);
  bool read_reg_(uint16_t reg, uint8_t *value);
  bool read_reg_16_(uint16_t reg, uint16_t *value);
  bool read_reg_32_(uint16_t reg, uint32_t *value);

  // Timing calculation helpers
  uint32_t calc_macro_period_(uint8_t vcsel_period);
  uint32_t timeout_mclks_to_microseconds_(uint32_t timeout_mclks, uint32_t macro_period_us);
  uint32_t timeout_microseconds_to_mclks_(uint32_t timeout_us, uint32_t macro_period_us);
  uint16_t encode_timeout_(uint32_t timeout_mclks);
  uint32_t decode_timeout_(uint16_t reg_val);

  DistanceMode distance_mode_{LONG};
  uint32_t timing_budget_ms_{50};
  uint32_t timeout_ms_{500};
  GPIOPin *enable_pin_{nullptr};

  // Sensor state
  bool measurement_started_{false};
  uint32_t start_time_ms_{0};
  uint16_t fast_osc_frequency_{0};
  uint16_t osc_calibrate_val_{0};

  // Multi-sensor support with enable pins
  static std::list<VL53L1XSensor *> vl53_sensors_;  // NOLINT
  static bool enable_pin_setup_complete_;           // NOLINT
};

}  // namespace vl53l1x
}  // namespace esphome
