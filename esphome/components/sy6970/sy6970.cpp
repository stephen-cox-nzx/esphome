#include "sy6970.h"
#include "esphome/core/hal.h"
#include "esphome/core/log.h"

// XPowersLib header
#include <XPowersLib.h>

namespace esphome {
namespace sy6970 {

static const char *const TAG = "sy6970";

// Static instance pointer for callbacks
SY6970Component *SY6970Component::instance_ = nullptr;

// Static callback functions
int SY6970Component::static_i2c_read(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint8_t len) {
  if (instance_ == nullptr)
    return -1;
  return instance_->i2c_read_reg(dev_addr, reg_addr, data, len);
}

int SY6970Component::static_i2c_write(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint8_t len) {
  if (instance_ == nullptr)
    return -1;
  return instance_->i2c_write_reg(dev_addr, reg_addr, data, len);
}

SY6970Component::SY6970Component() {
  instance_ = this;
}

SY6970Component::~SY6970Component() {
  if (this->pmu_ != nullptr) {
    delete this->pmu_;
    this->pmu_ = nullptr;
  }
  if (instance_ == this) {
    instance_ = nullptr;
  }
}

int SY6970Component::i2c_read_reg(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint8_t len) {
  // Use ESPHome's I2C methods
  i2c::ErrorCode err = this->write(&reg_addr, 1, false);
  if (err != i2c::ERROR_OK) {
    return -1;
  }
  err = this->read(data, len);
  if (err != i2c::ERROR_OK) {
    return -1;
  }
  return 0;
}

int SY6970Component::i2c_write_reg(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint8_t len) {
  // Use ESPHome's I2C methods
  // Need to write register address followed by data
  uint8_t buffer[len + 1];
  buffer[0] = reg_addr;
  memcpy(buffer + 1, data, len);
  
  i2c::ErrorCode err = this->write(buffer, len + 1);
  if (err != i2c::ERROR_OK) {
    return -1;
  }
  return 0;
}

void SY6970Component::setup() {
  ESP_LOGCONFIG(TAG, "Setting up SY6970...");

  // Initialize the XPowers library with callbacks
  this->pmu_ = new PowersSY6970(this->address_, static_i2c_read, static_i2c_write);
  if (!this->pmu_->init()) {
    ESP_LOGE(TAG, "Failed to initialize SY6970");
    this->mark_failed();
    return;
  }

  this->initialized_ = true;
  ESP_LOGCONFIG(TAG, "SY6970 initialized successfully");
}

void SY6970Component::dump_config() {
  ESP_LOGCONFIG(TAG, "SY6970:");
  LOG_I2C_DEVICE(this);
  if (this->is_failed()) {
    ESP_LOGE(TAG, "Communication with SY6970 failed!");
  } else if (this->pmu_ != nullptr) {
    ESP_LOGCONFIG(TAG, "  Chip: %s", this->pmu_->getChipName());
  }
}

void SY6970Component::loop() {
  // Regular updates can be handled by sensor components
}

uint16_t SY6970Component::get_vbus_voltage() {
  if (!this->initialized_ || this->pmu_ == nullptr)
    return 0;
  return this->pmu_->getVbusVoltage();
}

uint16_t SY6970Component::get_battery_voltage() {
  if (!this->initialized_ || this->pmu_ == nullptr)
    return 0;
  return this->pmu_->getBattVoltage();
}

uint16_t SY6970Component::get_system_voltage() {
  if (!this->initialized_ || this->pmu_ == nullptr)
    return 0;
  return this->pmu_->getSystemVoltage();
}

uint16_t SY6970Component::get_charge_current() {
  if (!this->initialized_ || this->pmu_ == nullptr)
    return 0;
  return this->pmu_->getChargeCurrent();
}

uint16_t SY6970Component::get_precharge_current() {
  if (!this->initialized_ || this->pmu_ == nullptr)
    return 0;
  return this->pmu_->getPrechargeCurr();
}

bool SY6970Component::is_vbus_connected() {
  if (!this->initialized_ || this->pmu_ == nullptr)
    return false;
  return this->pmu_->isVbusIn();
}

bool SY6970Component::is_charging() {
  if (!this->initialized_ || this->pmu_ == nullptr)
    return false;
  return this->pmu_->isCharging();
}

bool SY6970Component::is_charge_done() {
  if (!this->initialized_ || this->pmu_ == nullptr)
    return false;
  return this->pmu_->isChargeDone();
}

const char *SY6970Component::get_bus_status_string() {
  if (!this->initialized_ || this->pmu_ == nullptr)
    return "Unknown";
  return this->pmu_->getBusStatusString();
}

const char *SY6970Component::get_charge_status_string() {
  if (!this->initialized_ || this->pmu_ == nullptr)
    return "Unknown";
  return this->pmu_->getChargeStatusString();
}

const char *SY6970Component::get_ntc_status_string() {
  if (!this->initialized_ || this->pmu_ == nullptr)
    return "Unknown";
  return this->pmu_->getNTCStatusString();
}

uint8_t SY6970Component::get_bus_status() {
  if (!this->initialized_ || this->pmu_ == nullptr)
    return 0;
  return this->pmu_->getBusStatus();
}

uint8_t SY6970Component::get_charge_status() {
  if (!this->initialized_ || this->pmu_ == nullptr)
    return 0;
  return this->pmu_->chargeStatus();
}

uint8_t SY6970Component::get_ntc_status() {
  if (!this->initialized_ || this->pmu_ == nullptr)
    return 0;
  return this->pmu_->getNTCStatus();
}

void SY6970Component::set_input_current_limit(uint16_t milliamps) {
  if (!this->initialized_ || this->pmu_ == nullptr)
    return;
  this->pmu_->setInputCurrentLimit(milliamps);
}

void SY6970Component::set_charge_target_voltage(uint16_t millivolts) {
  if (!this->initialized_ || this->pmu_ == nullptr)
    return;
  this->pmu_->setChargeTargetVoltage(millivolts);
}

void SY6970Component::set_precharge_current(uint16_t milliamps) {
  if (!this->initialized_ || this->pmu_ == nullptr)
    return;
  this->pmu_->setPrechargeCurr(milliamps);
}

void SY6970Component::set_charge_current(uint16_t milliamps) {
  if (!this->initialized_ || this->pmu_ == nullptr)
    return;
  this->pmu_->setChargerConstantCurr(milliamps);
}

void SY6970Component::enable_charge() {
  if (!this->initialized_ || this->pmu_ == nullptr)
    return;
  this->pmu_->enableCharge();
}

void SY6970Component::disable_charge() {
  if (!this->initialized_ || this->pmu_ == nullptr)
    return;
  this->pmu_->disableCharge();
}

void SY6970Component::enable_stat_led() {
  if (!this->initialized_ || this->pmu_ == nullptr)
    return;
  this->pmu_->enableStatLed();
}

void SY6970Component::disable_stat_led() {
  if (!this->initialized_ || this->pmu_ == nullptr)
    return;
  this->pmu_->disableStatLed();
}

void SY6970Component::enable_adc_measure() {
  if (!this->initialized_ || this->pmu_ == nullptr)
    return;
  this->pmu_->enableADCMeasure();
}

uint16_t SY6970Component::get_charge_target_voltage() {
  if (!this->initialized_ || this->pmu_ == nullptr)
    return 0;
  return this->pmu_->getChargeTargetVoltage();
}

uint16_t SY6970Component::get_charge_constant_current() {
  if (!this->initialized_ || this->pmu_ == nullptr)
    return 0;
  return this->pmu_->getChargerConstantCurr();
}

}  // namespace sy6970
}  // namespace esphome
