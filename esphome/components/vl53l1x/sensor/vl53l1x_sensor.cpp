#include "vl53l1x_sensor.h"
#include "esphome/core/log.h"

namespace esphome {
namespace vl53l1x {

static const char *const TAG = "vl53l1x.sensor";

void VL53L1XSensor::setup() {
  ESP_LOGCONFIG(TAG, "Setting up VL53L1X Sensor...");
  this->last_measurement_time_ = 0;
}

void VL53L1XSensor::dump_config() {
  LOG_SENSOR("", "VL53L1X Distance Sensor", this);
  ESP_LOGCONFIG(TAG, "  Timeout: %u ms", this->timeout_ms_);
}

void VL53L1XSensor::on_distance(uint16_t distance_mm, RangeStatus status) {
  // Check if enough time has passed since last measurement (timeout-based throttling)
  uint32_t now = millis();
  if (now - this->last_measurement_time_ < this->timeout_ms_) {
    return;  // Skip this measurement, too soon
  }

  this->last_measurement_time_ = now;

  // Check range status
  if (status == RANGE_VALID || status == RANGE_VALID_MIN_RANGE_CLIPPED || status == RANGE_VALID_NO_WRAP_CHECK_FAIL) {
    float range_m = distance_mm / 1000.0f;
    ESP_LOGD(TAG, "Distance: %.3f m", range_m);
    this->publish_state(range_m);
  } else {
    ESP_LOGW(TAG, "Invalid range status: %d", status);
    this->publish_state(NAN);
  }
}

}  // namespace vl53l1x
}  // namespace esphome
