import setup_path 
import AutonomySim

import tempfile
import os
import numpy as np
import cv2
import pprint

# connect to the AutonomySim simulator
client = AutonomySim.MultirotorClient()
client.confirmConnection()

# add new vehicle
vehicle_name = "Drone2"
pose = AutonomySim.Pose(AutonomySim.Vector3r(0, 0, 0), AutonomySim.to_quaternion(0, 0, 0))

client.simAddVehicle(vehicle_name, "simpleflight", pose)
client.enableApiControl(True, vehicle_name)
client.armDisarm(True, vehicle_name)
client.takeoffAsync(10.0, vehicle_name)

requests = [AutonomySim.ImageRequest("0", AutonomySim.ImageType.DepthVis),  #depth visualization image
            AutonomySim.ImageRequest("1", AutonomySim.ImageType.DepthPerspective, True), #depth in perspective projection
            AutonomySim.ImageRequest("1", AutonomySim.ImageType.Scene), #scene vision image in png format
            AutonomySim.ImageRequest("1", AutonomySim.ImageType.Scene, False, False)]  #scene vision image in uncompressed RGBA array

responses = client.simGetImages(requests, vehicle_name=vehicle_name)
print('Retrieved images: %d' % len(responses))

tmp_dir = os.path.join(tempfile.gettempdir(), "AutonomySim_drone")
print ("Saving images to %s" % tmp_dir)
try:
    os.makedirs(tmp_dir)
except OSError:
    if not os.path.isdir(tmp_dir):
        raise

for idx, response in enumerate(responses):
    filename = os.path.join(tmp_dir, str(idx))

    if response.pixels_as_float:
        print("Type %d, size %d, pos %s" % (response.image_type, len(response.image_data_float), pprint.pformat(response.camera_position)))
        AutonomySim.write_pfm(os.path.normpath(filename + '.pfm'), AutonomySim.get_pfm_array(response))
    else:
        print("Type %d, size %d, pos %s" % (response.image_type, len(response.image_data_uint8), pprint.pformat(response.camera_position)))
        AutonomySim.write_file(os.path.normpath(filename + '.png'), response.image_data_uint8)