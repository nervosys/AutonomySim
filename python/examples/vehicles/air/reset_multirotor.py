import time

from autonomysim.clients import MultirotorClient
from autonomysim.utils import SetupPath


def main():
    SetupPath()

    # connect to the AutonomySim simulator
    client = MultirotorClient()
    client.confirmConnection()
    client.enableApiControl(True)
    client.armDisarm(True)

    print("fly")
    client.moveToPositionAsync(0, 0, -10, 5).join()

    print("reset")
    client.reset()
    client.enableApiControl(True)
    client.armDisarm(True)
    time.sleep(5)
    print("done")

    print("fly")
    client.moveToPositionAsync(0, 0, -10, 5).join()


if __name__ == "__main__":
    main()
