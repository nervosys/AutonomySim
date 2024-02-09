# In settings.json first activate computer vision mode:
# https://github.com/nervosys/AutonomySim/blob/master/docs/apis_image.md#computer-vision-mode

import setup_path
import autonomysim

import pprint
import os
import time
import math
import tempfile

pp = pprint.PrettyPrinter(indent=4)

client = autonomysim.VehicleClient()
client.confirmConnection()

autonomysim.wait_key("Press any key to set camera-0 gimbal to 15-degree pitch")
camera_pose = autonomysim.Pose(
    autonomysim.Vector3r(0, 0, 0), autonomysim.to_quaternion(math.radians(15), 0, 0)
)  # radians
client.simSetCameraPose("0", camera_pose)

autonomysim.wait_key("Press any key to get camera parameters")
for camera_name in range(5):
    camera_info = client.simGetCameraInfo(str(camera_name))
    print("CameraInfo %d:" % camera_name)
    pp.pprint(camera_info)

tmp_dir = os.path.join(tempfile.gettempdir(), "autonomysim_cv_mode")
print("Saving images to %s" % tmp_dir)
try:
    os.makedirs(tmp_dir)
except OSError:
    if not os.path.isdir(tmp_dir):
        raise

autonomysim.wait_key("Press any key to get images")
for x in range(3):  # do few times
    z = x * -20 - 5  # some random number
    client.simSetVehiclePose(
        autonomysim.Pose(
            autonomysim.Vector3r(z, z, z),
            autonomysim.to_quaternion(x / 3.0, 0, x / 3.0),
        ),
        True,
    )

    responses = client.simGetImages(
        [
            autonomysim.ImageRequest("0", autonomysim.ImageType.DepthVis),
            autonomysim.ImageRequest("1", autonomysim.ImageType.DepthPerspective, True),
            autonomysim.ImageRequest("2", autonomysim.ImageType.Segmentation),
            autonomysim.ImageRequest("3", autonomysim.ImageType.Scene),
            autonomysim.ImageRequest("4", autonomysim.ImageType.DisparityNormalized),
            autonomysim.ImageRequest("4", autonomysim.ImageType.SurfaceNormals),
        ]
    )

    for i, response in enumerate(responses):
        filename = os.path.join(tmp_dir, str(x) + "_" + str(i))
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

    pose = client.simGetVehiclePose()
    pp.pprint(pose)

    time.sleep(3)

# currently reset() doesn't work in CV mode. Below is the workaround
client.simSetVehiclePose(
    autonomysim.Pose(autonomysim.Vector3r(0, 0, 0), autonomysim.to_quaternion(0, 0, 0)),
    True,
)
