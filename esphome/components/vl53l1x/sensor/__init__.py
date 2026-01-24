import esphome.codegen as cg
from esphome.components import sensor
import esphome.config_validation as cv
from esphome.const import (
    CONF_ID,
    CONF_TIMEOUT,
    ICON_ARROW_EXPAND_VERTICAL,
    STATE_CLASS_MEASUREMENT,
    UNIT_METER,
)

from .. import CONF_VL53L1X_ID, VL53L1XComponent, vl53l1x_ns

DEPENDENCIES = ["vl53l1x"]

VL53L1XSensor = vl53l1x_ns.class_(
    "VL53L1XSensor", sensor.Sensor, cg.Component, cg.Parented.template(VL53L1XComponent)
)


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
            cv.Optional(CONF_TIMEOUT, default="500ms"): cv.All(
                cv.positive_time_period_milliseconds,
                cv.Range(max=cv.TimePeriod(milliseconds=60000)),
            ),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await sensor.register_sensor(var, config)
    await cg.register_component(var, config)
    await cg.register_parented(var, config[CONF_VL53L1X_ID])

    cg.add(var.set_timeout(config[CONF_TIMEOUT].total_milliseconds))

    # Register sensor as listener with parent
    parent = await cg.get_variable(config[CONF_VL53L1X_ID])
    cg.add(parent.register_listener(var))
