"""
For connecting to the AutonomySim drone environment and testing API functionality
"""
import setup_path
import autonomysim

import os
import tempfile
import pprint

# connect to the AutonomySim simulator
client = autonomysim.MultirotorClient()
client.confirmConnection()
client.enableApiControl(True)
client.armDisarm(True)

state = client.getMultirotorState()
s = pprint.pformat(state)
print("state: %s" % s)

client.moveByManualAsync(vx_max=1e6, vy_max=1e6, z_min=-1e6, duration=1e10)
autonomysim.wait_key("Manual mode is setup. Press any key to send RC data to takeoff")

client.moveByRC(
    rcdata=autonomysim.RCData(
        pitch=0.0, throttle=1.0, is_initialized=True, is_valid=True
    )
)

autonomysim.wait_key("Set Yaw and pitch to 0.5")

client.moveByRC(
    rcdata=autonomysim.RCData(
        roll=0.5, throttle=1.0, yaw=0.5, is_initialized=True, is_valid=True
    )
)
