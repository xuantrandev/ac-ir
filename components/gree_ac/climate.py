"""
ESPHome external component: gree_ac
Climate platform for Gree AC units using IR remote protocol.

Protocol reverse-engineered from raw IR captures:
  - 4-byte Frame 1: mode, temperature, fan speed, swing, power
  - 4-byte Frame 2: swing echo, fixed bytes, checksum
Both frames sent in sequence; Frame 2 has no header mark.
"""

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import climate, remote_base
from esphome.const import CONF_ID

DEPENDENCIES = ["climate", "remote_receiver", "remote_transmitter"]
AUTO_LOAD = ["remote_base"]

gree_ac_ns = cg.esphome_ns.namespace("gree_ac")

GreeAcClimate = gree_ac_ns.class_(
    "GreeAcClimate",
    climate.Climate,
    cg.Component,
)

CONF_RECEIVER_ID = "receiver_id"
CONF_TRANSMITTER_ID = "transmitter_id"

remote_base_ns = cg.esphome_ns.namespace("remote_base")
RemoteReceiverBase = remote_base_ns.class_("RemoteReceiverBase")
RemoteTransmitterBase = remote_base_ns.class_("RemoteTransmitterBase")

CONFIG_SCHEMA = climate.CLIMATE_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(GreeAcClimate),
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
