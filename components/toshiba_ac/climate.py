"""
ESPHome external component: toshiba_ac
Climate platform for Toshiba AC units using IR remote protocol.

Protocol reverse-engineered from raw IR captures:
  - 9-byte settings packet: mode, temperature, fan speed
  - 7-byte swing packet: swing on/off
Both packet types are sent twice (with a ~5.5ms repeat gap).
"""

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import climate, remote_base
from esphome.const import CONF_ID

DEPENDENCIES = ["climate", "remote_receiver", "remote_transmitter"]
AUTO_LOAD = ["remote_base"]

toshiba_ac_ns = cg.esphome_ns.namespace("toshiba_ac")

ToshibaAcClimate = toshiba_ac_ns.class_(
    "ToshibaAcClimate",
    climate.Climate,
    cg.Component,
)

CONF_RECEIVER_ID = "receiver_id"
CONF_TRANSMITTER_ID = "transmitter_id"

# C++ types from remote_base namespace
remote_base_ns = cg.esphome_ns.namespace("remote_base")
RemoteReceiverBase = remote_base_ns.class_("RemoteReceiverBase")
RemoteTransmitterBase = remote_base_ns.class_("RemoteTransmitterBase")

CONFIG_SCHEMA = climate.CLIMATE_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(ToshibaAcClimate),
        cv.Required(CONF_RECEIVER_ID): cv.use_id(RemoteReceiverBase),
        cv.Required(CONF_TRANSMITTER_ID): cv.use_id(RemoteTransmitterBase),
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await climate.register_climate(var, config)

    receiver = await cg.get_variable(config[CONF_RECEIVER_ID])
    cg.add(receiver.register_listener(var))

    transmitter = await cg.get_variable(config[CONF_TRANSMITTER_ID])
    cg.add(var.set_transmitter(transmitter))
