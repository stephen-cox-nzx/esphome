from esphome import pins
import esphome.codegen as cg
from esphome.components import i2c
import esphome.config_validation as cv
from esphome.const import CONF_ID, CONF_ENABLE_PIN

CODEOWNERS = ["@esphome/core"]
DEPENDENCIES = ["i2c"]
MULTI_CONF = True

vl53l1x_ns = cg.esphome_ns.namespace("vl53l1x")
VL53L1XComponent = vl53l1x_ns.class_("VL53L1XComponent", cg.Component, i2c.I2CDevice)

CONF_VL53L1X_ID = "vl53l1x_id"
CONF_DISTANCE_MODE = "distance_mode"
CONF_TIMING_BUDGET = "timing_budget"

DistanceMode = vl53l1x_ns.enum("DistanceMode")
DISTANCE_MODES = {
    "SHORT": DistanceMode.SHORT,
    "MEDIUM": DistanceMode.MEDIUM,
    "LONG": DistanceMode.LONG,
}

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(VL53L1XComponent),
            cv.Optional(CONF_DISTANCE_MODE, default="LONG"): cv.enum(
                DISTANCE_MODES, upper=True
            ),
            cv.Optional(CONF_TIMING_BUDGET, default="50ms"): cv.All(
                cv.positive_time_period_milliseconds,
                cv.Range(
                    min=cv.TimePeriod(milliseconds=20),
                    max=cv.TimePeriod(milliseconds=1000),
                ),
            ),
            cv.Optional(CONF_ENABLE_PIN): pins.gpio_output_pin_schema,
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
    .extend(i2c.i2c_device_schema(0x29))
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)

    cg.add(var.set_distance_mode(config[CONF_DISTANCE_MODE]))
    cg.add(var.set_timing_budget(config[CONF_TIMING_BUDGET].total_milliseconds))

    if CONF_ENABLE_PIN in config:
        enable = await cg.gpio_pin_expression(config[CONF_ENABLE_PIN])
        cg.add(var.set_enable_pin(enable))
