#pragma once

#include "esphome/core/component.h"
#include "esphome/core/hal.h"
#include "esphome/components/sensor/sensor.h"

#include "../vl53l1x.h"

namespace esphome {
namespace vl53l1x {

class VL53L1XSensor : public sensor::Sensor,
                      public Component,
                      public Parented<VL53L1XComponent>,
                      public VL53L1XListener {
 public:
  void setup() override;
  void dump_config() override;

  void set_timeout(uint32_t timeout_ms) { this->timeout_ms_ = timeout_ms; }

  // Implement VL53L1XListener interface
  void on_distance(uint16_t distance_mm, RangeStatus status) override;

 protected:
  uint32_t timeout_ms_{500};
  uint32_t last_measurement_time_{0};
};

}  // namespace vl53l1x
}  // namespace esphome
