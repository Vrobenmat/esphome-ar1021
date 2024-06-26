import esphome.codegen as cg
import esphome.config_validation as cv

from esphome import pins
from esphome.components import i2c, touchscreen
from esphome.const import CONF_ID, CONF_INTERRUPT_PIN

CODEOWNERS = ["@vrobenmat"]
DEPENDENCIES = ["i2c"]

ar1021_ns = cg.esphome_ns.namespace("ar1021")

AR1021Component = ar1021_ns.class_(
    "AR1021Component",
    touchscreen.Touchscreen,
    i2c.I2CDevice,
)

CONF_AR1021_ID = "ar1021_id"

CONFIG_SCHEMA = touchscreen.TOUCHSCREEN_SCHEMA.extend(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(AR1021Component),
            cv.Required(CONF_INTERRUPT_PIN): cv.All(
                pins.internal_gpio_input_pin_schema
            ),
        }
    ).extend(i2c.i2c_device_schema(0x4D))
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await touchscreen.register_touchscreen(var, config)
    await i2c.register_i2c_device(var, config)

    interrupt_pin = await cg.gpio_pin_expression(config[CONF_INTERRUPT_PIN])
    cg.add(var.set_interrupt_pin(interrupt_pin))