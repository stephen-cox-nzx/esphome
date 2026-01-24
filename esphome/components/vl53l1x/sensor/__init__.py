from esphome import pins
import esphome.codegen as cg
from esphome.components import sensor
import esphome.config_validation as cv
from esphome.const import (
    CONF_ENABLE_PIN,
    CONF_ID,
    CONF_TIMEOUT,
    ICON_ARROW_EXPAND_VERTICAL,
    STATE_CLASS_MEASUREMENT,
    UNIT_METER,
)

from .. import CONF_VL53L1X_ID, VL53L1XComponent, vl53l1x_ns

DEPENDENCIES = ["vl53l1x"]

VL53L1XSensor = vl53l1x_ns.class_(
    "VL53L1XSensor", sensor.Sensor, cg.PollingComponent, cg.Parented.template(VL53L1XComponent)
)

DistanceMode = vl53l1x_ns.enum("DistanceMode")
DISTANCE_MODES = {
    "SHORT": DistanceMode.SHORT,
    "MEDIUM": DistanceMode.MEDIUM,
    "LONG": DistanceMode.LONG,
}

CONF_DISTANCE_MODE = "distance_mode"
CONF_TIMING_BUDGET = "timing_budget"


CONFIG_SCHEMA = (
    sensor.sensor_schema(
        VL53L1XSensor,
        unit_of_measurement=UNIT_METER,
        icon=ICON_ARROW_EXPAND_VERTICAL,
        accuracy_decimals=3,
        state_class=STATE_CLASS_MEASUREMENT,
    )
    .extend(
        {
            cv.GenerateID(CONF_VL53L1X_ID): cv.use_id(VL53L1XComponent),
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
            cv.Optional(CONF_TIMEOUT, default="500ms"): cv.All(
                cv.positive_time_period_milliseconds,
                cv.Range(max=cv.TimePeriod(milliseconds=60000)),
            ),
            cv.Optional(CONF_ENABLE_PIN): pins.gpio_output_pin_schema,
        }
    )
    .extend(cv.polling_component_schema("60s"))
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await sensor.register_sensor(var, config)
    await cg.register_component(var, config)
    await cg.register_parented(var, config[CONF_VL53L1X_ID])

    cg.add(var.set_distance_mode(config[CONF_DISTANCE_MODE]))
    cg.add(var.set_timing_budget(config[CONF_TIMING_BUDGET].total_milliseconds))
    cg.add(var.set_timeout(config[CONF_TIMEOUT].total_milliseconds))

    if CONF_ENABLE_PIN in config:
        enable = await cg.gpio_pin_expression(config[CONF_ENABLE_PIN])
        cg.add(var.set_enable_pin(enable))
