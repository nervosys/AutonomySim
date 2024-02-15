import pprint
import os
import time
import math
import tempfile

from autonomysim.clients import VehicleClient
from autonomysim.types import Vector3r, Pose, ImageRequest, ImageType
from autonomysim.utils import wait_key, SetupPath
from autonomysim.utils.convs import to_quaternion
from python.autonomysim.utils.io import get_pfm_array, write_file


def main():
    # In settings.json first activate computer vision mode:
    # https://github.com/nervosys/AutonomySim/blob/master/docs/apis_image.md#computer-vision-mode

    SetupPath()

    pp = pprint.PrettyPrinter(indent=4)

    client = VehicleClient()
    client.confirmConnection()

    wait_key("Press any key to set camera-0 gimbal to 15-degree pitch")
    camera_pose = Pose(
        Vector3r(0, 0, 0), to_quaternion(math.radians(15), 0, 0)
    )  # radians
    client.simSetCameraPose("0", camera_pose)

    wait_key("Press any key to get camera parameters")
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

    wait_key("Press any key to get images")
    for x in range(3):  # do few times
        z = x * -20 - 5  # some random number
        client.simSetVehiclePose(
            Pose(
                Vector3r(z, z, z),
                to_quaternion(x / 3.0, 0, x / 3.0),
            ),
            True,
        )

        responses = client.simGetImages(
            [
                ImageRequest("0", ImageType.DepthVis),
                ImageRequest("1", ImageType.DepthPerspective, True),
                ImageRequest("2", ImageType.Segmentation),
                ImageRequest("3", ImageType.Scene),
                ImageRequest("4", ImageType.DisparityNormalized),
                ImageRequest("4", ImageType.SurfaceNormals),
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
                    os.path.normpath(filename + ".pfm"),
                    get_pfm_array(response),
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
                write_file(
                    os.path.normpath(filename + ".png"), response.image_data_uint8
                )

        pose = client.simGetVehiclePose()
        pp.pprint(pose)

        time.sleep(3)

    # currently reset() doesn't work in CV mode. Below is the workaround
    client.simSetVehiclePose(
        Pose(Vector3r(0, 0, 0), to_quaternion(0, 0, 0)),
        True,
    )


if __name__ == "__main__":
    main()
