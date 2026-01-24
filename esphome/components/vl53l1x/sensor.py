from esphome import pins
import esphome.codegen as cg
from esphome.components import i2c, sensor
import esphome.config_validation as cv
from esphome.const import (
    CONF_ADDRESS,
    CONF_ENABLE_PIN,
    CONF_TIMEOUT,
    ICON_ARROW_EXPAND_VERTICAL,
    STATE_CLASS_MEASUREMENT,
    UNIT_METER,
)

DEPENDENCIES = ["i2c"]

vl53l1x_ns = cg.esphome_ns.namespace("vl53l1x")
VL53L1XSensor = vl53l1x_ns.class_(
    "VL53L1XSensor", sensor.Sensor, cg.PollingComponent, i2c.I2CDevice
)

DistanceMode = vl53l1x_ns.enum("DistanceMode")
DISTANCE_MODES = {
    "SHORT": DistanceMode.SHORT,
    "MEDIUM": DistanceMode.MEDIUM,
    "LONG": DistanceMode.LONG,
}

CONF_DISTANCE_MODE = "distance_mode"
CONF_TIMING_BUDGET = "timing_budget"


def check_keys(obj):
    if obj[CONF_ADDRESS] != 0x29 and CONF_ENABLE_PIN not in obj:
        msg = "Address other than 0x29 requires enable_pin definition to allow sensor "
        msg += "re-addressing. Also if you have more than one VL53L1X device on the same "
        msg += "i2c bus, then all VL53L1X devices must have enable_pin defined."
        raise cv.Invalid(msg)
    return obj


def check_timeout(value):
    value = cv.positive_time_period_milliseconds(value)
    if value.total_milliseconds > 60000:
        raise cv.Invalid("Maximum timeout can not be greater than 60 seconds")
    return value


def check_timing_budget(value):
    value = cv.positive_time_period_milliseconds(value)
    if value.total_milliseconds < 20:
        raise cv.Invalid("Minimum timing budget is 20 ms")
    if value.total_milliseconds > 1000:
        raise cv.Invalid("Maximum timing budget is 1000 ms")
    return value


CONFIG_SCHEMA = cv.All(
    sensor.sensor_schema(
        VL53L1XSensor,
        unit_of_measurement=UNIT_METER,
        icon=ICON_ARROW_EXPAND_VERTICAL,
        accuracy_decimals=3,
        state_class=STATE_CLASS_MEASUREMENT,
    )
    .extend(
        {
            cv.Optional(CONF_DISTANCE_MODE, default="LONG"): cv.enum(
                DISTANCE_MODES, upper=True
            ),
            cv.Optional(CONF_TIMING_BUDGET, default="50ms"): check_timing_budget,
            cv.Optional(CONF_TIMEOUT, default="500ms"): check_timeout,
            cv.Optional(CONF_ENABLE_PIN): pins.gpio_output_pin_schema,
        }
    )
    .extend(cv.polling_component_schema("60s"))
    .extend(i2c.i2c_device_schema(0x29)),
    check_keys,
)


async def to_code(config):
    var = await sensor.new_sensor(config)
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)

    cg.add(var.set_distance_mode(config[CONF_DISTANCE_MODE]))
    cg.add(var.set_timing_budget(config[CONF_TIMING_BUDGET].total_milliseconds))
    cg.add(var.set_timeout(config[CONF_TIMEOUT].total_milliseconds))

    if CONF_ENABLE_PIN in config:
        enable = await cg.gpio_pin_expression(config[CONF_ENABLE_PIN])
        cg.add(var.set_enable_pin(enable))
