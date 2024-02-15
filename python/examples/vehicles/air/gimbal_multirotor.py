import time

from autonomysim.types import Pose, Vector3r
from autonomysim.utils import SetupPath
from autonomysim.utils.convs import to_quaternion
from autonomysim.clients import MultirotorClient


def main():
    SetupPath()

    # connect to the AutonomySim simulator
    client = MultirotorClient()
    client.confirmConnection()
    client.enableApiControl(True)
    client.armDisarm(True)

    # MultirotorClient.wait_key('Press any key to takeoff')
    print("Taking off")
    client.takeoffAsync().join()
    print("Ready")

    for i in range(5):
        client.moveToPositionAsync(
            float(-50.00), float(50.26), float(-20.58), float(3.5)
        )
        time.sleep(6)
        camera_pose = Pose(Vector3r(0, 0, 0), to_quaternion(0.5, 0.5, 0.1))
        client.simSetCameraPose("0", camera_pose)
        client.moveToPositionAsync(
            float(50.00), float(-50.26), float(-10.58), float(3.5)
        )
        time.sleep(6)
        camera_pose = Pose(Vector3r(0, 0, 0), to_quaternion(-0.5, -0.5, -0.1))
        client.simSetCameraPose("0", camera_pose)


if __name__ == "__main__":
    main()
