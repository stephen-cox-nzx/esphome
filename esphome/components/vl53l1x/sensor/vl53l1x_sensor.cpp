#include "vl53l1x_sensor.h"
#include "esphome/core/log.h"

namespace esphome {
namespace vl53l1x {

static const char *const TAG = "vl53l1x.sensor";

void VL53L1XSensor::setup() {
  ESP_LOGCONFIG(TAG, "Setting up VL53L1X Sensor...");

  // Set enable pin on parent if provided
  if (this->enable_pin_ != nullptr) {
    this->parent_->set_enable_pin(this->enable_pin_);
  }

  // Configure distance mode
  if (!this->parent_->set_distance_mode(this->distance_mode_)) {
    ESP_LOGE(TAG, "Failed to set distance mode");
    this->mark_failed();
    return;
  }

  // Set measurement timing budget
  if (!this->parent_->set_measurement_timing_budget(this->timing_budget_ms_ * 1000)) {
    ESP_LOGE(TAG, "Failed to set timing budget");
    this->mark_failed();
    return;
  }

  ESP_LOGCONFIG(TAG, "VL53L1X Sensor setup complete");
}

void VL53L1XSensor::dump_config() {
  LOG_SENSOR("", "VL53L1X Distance Sensor", this);
  LOG_UPDATE_INTERVAL(this);

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
  ESP_LOGCONFIG(TAG, "  Timeout: %u ms", this->timeout_ms_);

  if (this->is_failed()) {
    ESP_LOGE(TAG, "VL53L1X Sensor failed to initialize");
  }
}

void VL53L1XSensor::update() {
  if (this->measurement_started_) {
    ESP_LOGW(TAG, "Measurement already in progress");
    this->publish_state(NAN);
    return;
  }

  if (!this->parent_->start_measurement()) {
    ESP_LOGW(TAG, "Failed to start measurement");
    this->publish_state(NAN);
    return;
  }

  this->measurement_started_ = true;
  this->start_time_ms_ = millis();
}

void VL53L1XSensor::loop() {
  if (!this->measurement_started_) {
    return;
  }

  // Check for timeout
  if (millis() - this->start_time_ms_ > this->timeout_ms_) {
    ESP_LOGW(TAG, "Measurement timeout");
    this->measurement_started_ = false;
    this->publish_state(NAN);
    return;
  }

  // Check if data is ready
  if (!this->parent_->data_ready()) {
    return;
  }

  // Read the range
  uint16_t range_mm = this->parent_->read_range_mm();
  RangeStatus status = this->parent_->get_range_status();

  // Clear interrupt
  this->parent_->write_reg(SYSTEM__INTERRUPT_CLEAR, 0x01);

  this->measurement_started_ = false;

  // Check range status
  if (status == RANGE_VALID || status == RANGE_VALID_MIN_RANGE_CLIPPED || status == RANGE_VALID_NO_WRAP_CHECK_FAIL) {
    float range_m = range_mm / 1000.0f;
    ESP_LOGD(TAG, "Distance: %.3f m", range_m);
    this->publish_state(range_m);
  } else {
    ESP_LOGW(TAG, "Invalid range status: %d", status);
    this->publish_state(NAN);
  }
}

}  // namespace vl53l1x
}  // namespace esphome
