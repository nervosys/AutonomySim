import setup_path
import autonomysim

import tempfile
import os
import numpy as np
import cv2
import pprint

# connect to the AutonomySim simulator
client = autonomysim.MultirotorClient()
client.confirmConnection()

# add new vehicle
vehicle_name = "Drone2"
pose = autonomysim.Pose(
    autonomysim.Vector3r(0, 0, 0), autonomysim.to_quaternion(0, 0, 0)
)

client.simAddVehicle(vehicle_name, "simpleflight", pose)
client.enableApiControl(True, vehicle_name)
client.armDisarm(True, vehicle_name)
client.takeoffAsync(10.0, vehicle_name)

requests = [
    autonomysim.ImageRequest(
        "0", autonomysim.ImageType.DepthVis
    ),  # depth visualization image
    autonomysim.ImageRequest(
        "1", autonomysim.ImageType.DepthPerspective, True
    ),  # depth in perspective projection
    autonomysim.ImageRequest(
        "1", autonomysim.ImageType.Scene
    ),  # scene vision image in png format
    autonomysim.ImageRequest("1", autonomysim.ImageType.Scene, False, False),
]  # scene vision image in uncompressed RGBA array

responses = client.simGetImages(requests, vehicle_name=vehicle_name)
print("Retrieved images: %d" % len(responses))

tmp_dir = os.path.join(tempfile.gettempdir(), "autonomysim_drone")
print("Saving images to %s" % tmp_dir)
try:
    os.makedirs(tmp_dir)
except OSError:
    if not os.path.isdir(tmp_dir):
        raise

for idx, response in enumerate(responses):
    filename = os.path.join(tmp_dir, str(idx))

    if response.pixels_as_float:
        print(
            "Type %d, size %d, pos %s"
            % (
                response.image_type,
                len(response.image_data_float),
                pprint.pformat(response.camera_position),
            )
        )
        autonomysim.write_pfm(
            os.path.normpath(filename + ".pfm"), autonomysim.get_pfm_array(response)
        )
    else:
        print(
            "Type %d, size %d, pos %s"
            % (
                response.image_type,
                len(response.image_data_uint8),
                pprint.pformat(response.camera_position),
            )
        )
        autonomysim.write_file(
            os.path.normpath(filename + ".png"), response.image_data_uint8
        )
