#pragma once

#include "esphome/components/i2c/i2c.h"
#include "esphome/core/component.h"

// Forward declare the PowersSY6970 class
class PowersSY6970;

namespace esphome {
namespace sy6970 {

class SY6970Component : public Component, public i2c::I2CDevice {
 public:
  SY6970Component();
  ~SY6970Component();

  void setup() override;
  void dump_config() override;
  void loop() override;
  float get_setup_priority() const override { return setup_priority::DATA; }

  // Get voltage readings
  uint16_t get_vbus_voltage();
  uint16_t get_battery_voltage();
  uint16_t get_system_voltage();

  // Get current readings
  uint16_t get_charge_current();
  uint16_t get_precharge_current();

  // Get status
  bool is_vbus_connected();
  bool is_charging();
  bool is_charge_done();
  const char *get_bus_status_string();
  const char *get_charge_status_string();
  const char *get_ntc_status_string();
  uint8_t get_bus_status();
  uint8_t get_charge_status();
  uint8_t get_ntc_status();

  // Configuration methods
  void set_input_current_limit(uint16_t milliamps);
  void set_charge_target_voltage(uint16_t millivolts);
  void set_precharge_current(uint16_t milliamps);
  void set_charge_current(uint16_t milliamps);
  void enable_charge();
  void disable_charge();
  void enable_stat_led();
  void disable_stat_led();
  void enable_adc_measure();

  // Get configuration values
  uint16_t get_charge_target_voltage();
  uint16_t get_charge_constant_current();

  // I2C callback functions for XPowersLib
  int i2c_read_reg(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint8_t len);
  int i2c_write_reg(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint8_t len);

 protected:
  PowersSY6970 *pmu_{nullptr};
  bool initialized_{false};
  static SY6970Component *instance_;
  static int static_i2c_read(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint8_t len);
  static int static_i2c_write(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint8_t len);
};
