# In settings.json first activate computer vision mode:
# https://github.com/nervosys/AutonomySim/blob/master/docs/apis_image.md#computer-vision-mode

import setup_path
import autonomysim

import pprint
import time
import cv2  # conda install opencv

client = autonomysim.VehicleClient()
client.confirmConnection()

print("Time,Speed,Gear,PX,PY,PZ,OW,OX,OY,OZ")

# monitor car state while you drive it manually.
while (cv2.waitKey(1) & 0xFF) == 0xFF:
    kinematics = client.simGetGroundTruthKinematics()
    environment = client.simGetGroundTruthEnvironment()

    print(
        "Kinematics: %s\nEnvironemt %s"
        % (pprint.pformat(kinematics), pprint.pformat(environment))
    )
    time.sleep(1)
