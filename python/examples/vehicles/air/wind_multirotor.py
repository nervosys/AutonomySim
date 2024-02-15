import time

from autonomysim.types import Vector3r, LandedState
from autonomysim.clients import MultirotorClient
from autonomysim.utils import SetupPath


def main():
    SetupPath()

    client = MultirotorClient()
    client.confirmConnection()
    client.enableApiControl(True)

    client.armDisarm(True)

    print("Setting wind to 10m/s in forward direction")  # NED
    wind = Vector3r(10, 0, 0)
    client.simSetWind(wind)

    # Takeoff or hover
    landed = client.getMultirotorState().landed_state
    if landed == LandedState.Landed:
        print("taking off...")
        client.takeoffAsync().join()
    else:
        print("already flying...")
        client.hoverAsync().join()

    time.sleep(5)

    print("Setting wind to 15m/s towards right")  # NED
    wind = Vector3r(0, 15, 0)
    client.simSetWind(wind)

    time.sleep(5)

    # Set wind to 0
    print("Resetting wind to 0")
    wind = Vector3r(0, 0, 0)
    client.simSetWind(wind)


if __name__ == "__main__":
    main()
