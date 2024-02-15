import time

from autonomysim.clients import MultirotorClient
from autonomysim.utils import SetupPath


def main():
    SetupPath()

    client = MultirotorClient()
    client.confirmConnection()

    pose = client.simGetVehiclePose()

    # teleport the drone + 10 meters in x-direction
    pose.position.x_val += 10

    client.simSetVehiclePose(pose, True, "PX4")

    time.sleep(2)

    # teleport the drone back
    pose.position.x_val -= 10

    client.simSetVehiclePose(pose, True, "PX4")


if __name__ == "__main__":
    main()
