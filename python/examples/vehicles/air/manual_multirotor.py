import pprint

from autonomysim.types import RCData
from autonomysim.utils import wait_key, SetupPath
from autonomysim.clients import MultirotorClient


def main():
    """
    Connect to the AutonomySim drone environment and test API functionality
    """
    SetupPath()

    # connect to the AutonomySim simulator
    client = MultirotorClient()
    client.confirmConnection()
    client.enableApiControl(True)
    client.armDisarm(True)

    state = client.getMultirotorState()
    s = pprint.pformat(state)
    print("state: %s" % s)

    client.moveByManualAsync(vx_max=1e6, vy_max=1e6, z_min=-1e6, duration=1e10)
    wait_key("Manual mode is setup. Press any key to send RC data to takeoff")

    client.moveByRC(
        rcdata=RCData(pitch=0.0, throttle=1.0, is_initialized=True, is_valid=True)
    )
    wait_key("Set Yaw and pitch to 0.5")
    client.moveByRC(
        rcdata=RCData(
            roll=0.5, throttle=1.0, yaw=0.5, is_initialized=True, is_valid=True
        )
    )


if __name__ == "__main__":
    main()
