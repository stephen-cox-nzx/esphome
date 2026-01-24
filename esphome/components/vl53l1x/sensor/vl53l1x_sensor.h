#pragma once

#include "esphome/core/component.h"
#include "esphome/core/hal.h"
#include "esphome/components/sensor/sensor.h"

#include "../vl53l1x.h"

namespace esphome {
namespace vl53l1x {

class VL53L1XSensor : public sensor::Sensor, public PollingComponent, public Parented<VL53L1XComponent> {
 public:
  void setup() override;
  void dump_config() override;
  void update() override;
  void loop() override;

  void set_distance_mode(DistanceMode mode) { this->distance_mode_ = mode; }
  void set_timing_budget(uint32_t timing_budget_ms) { this->timing_budget_ms_ = timing_budget_ms; }
  void set_timeout(uint32_t timeout_ms) { this->timeout_ms_ = timeout_ms; }
  void set_enable_pin(GPIOPin *enable) { this->enable_pin_ = enable; }

 protected:
  DistanceMode distance_mode_{LONG};
  uint32_t timing_budget_ms_{50};
  uint32_t timeout_ms_{500};
  GPIOPin *enable_pin_{nullptr};

  // Sensor state
  bool measurement_started_{false};
  uint32_t start_time_ms_{0};
};

}  // namespace vl53l1x
}  // namespace esphome
