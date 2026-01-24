import esphome.codegen as cg
from esphome.components import i2c
import esphome.config_validation as cv
from esphome.const import CONF_ID

CODEOWNERS = ["@esphome/core"]
DEPENDENCIES = ["i2c"]
MULTI_CONF = True

vl53l1x_ns = cg.esphome_ns.namespace("vl53l1x")
VL53L1XComponent = vl53l1x_ns.class_("VL53L1XComponent", cg.Component, i2c.I2CDevice)

CONF_VL53L1X_ID = "vl53l1x_id"

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(VL53L1XComponent),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
    .extend(i2c.i2c_device_schema(0x29))
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)
