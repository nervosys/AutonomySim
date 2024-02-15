import time
import pprint
import cv2

from autonomysim.clients import VehicleClient
from autonomysim.utils import SetupPath


def main():
    # In settings.json first activate computer vision mode:
    # https://github.com/nervosys/AutonomySim/blob/master/docs/apis_image.md#computer-vision-mode

    SetupPath()

    client = VehicleClient()
    client.confirmConnection()

    print("Time,Speed,Gear,PX,PY,PZ,OW,OX,OY,OZ")

    # monitor vehicle state while you drive it manually.
    while (cv2.waitKey(1) & 0xFF) == 0xFF:
        kinematics = client.simGetGroundTruthKinematics()
        environment = client.simGetGroundTruthEnvironment()
        print(
            "Kinematics: %s\nEnvironemt %s"
            % (pprint.pformat(kinematics), pprint.pformat(environment))
        )
        time.sleep(1)


if __name__ == "__main__":
    main()
