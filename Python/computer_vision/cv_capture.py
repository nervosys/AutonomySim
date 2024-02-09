# In settings.json first activate computer vision mode:
# https://github.com/nervosys/AutonomySim/blob/master/docs/apis_image.md#computer-vision-mode

import setup_path
import autonomysim

import pprint
import tempfile
import os
import time

pp = pprint.PrettyPrinter(indent=4)

client = autonomysim.VehicleClient()

autonomysim.wait_key("Press any key to get camera parameters")
for camera_id in range(2):
    camera_info = client.simGetCameraInfo(str(camera_id))
    print("CameraInfo %d: %s" % (camera_id, pp.pprint(camera_info)))

autonomysim.wait_key("Press any key to get images")
tmp_dir = os.path.join(tempfile.gettempdir(), "autonomysim_drone")
print("Saving images to %s" % tmp_dir)
try:
    for n in range(3):
        os.makedirs(os.path.join(tmp_dir, str(n)))
except OSError:
    if not os.path.isdir(tmp_dir):
        raise

for x in range(50):  # do few times
    # xn = 1 + x*5  # some random number
    client.simSetVehiclePose(
        autonomysim.Pose(
            autonomysim.Vector3r(x, 0, -2), autonomysim.to_quaternion(0, 0, 0)
        ),
        True,
    )
    time.sleep(0.1)

    responses = client.simGetImages(
        [
            autonomysim.ImageRequest("0", autonomysim.ImageType.Scene),
            autonomysim.ImageRequest("1", autonomysim.ImageType.Scene),
            autonomysim.ImageRequest("2", autonomysim.ImageType.Scene),
        ]
    )

    for i, response in enumerate(responses):
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
                os.path.normpath(os.path.join(tmp_dir, str(x) + "_" + str(i) + ".pfm")),
                autonomysim.get_pfm_array(response),
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
                os.path.normpath(
                    os.path.join(tmp_dir, str(i), str(x) + "_" + str(i) + ".png")
                ),
                response.image_data_uint8,
            )

    pose = client.simGetVehiclePose()
    pp.pprint(pose)

    time.sleep(3)

# currently reset() doesn't work in CV mode. Below is the workaround
client.simSetVehiclePose(
    autonomysim.Pose(autonomysim.Vector3r(0, 0, 0), autonomysim.to_quaternion(0, 0, 0)),
    True,
)
