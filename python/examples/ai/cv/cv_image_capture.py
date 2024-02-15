import os
import time
import pprint
import tempfile

from autonomysim.clients import VehicleClient
from autonomysim.types import Pose, Vector3r, ImageRequest, ImageType
from autonomysim.utils import wait_key, SetupPath
from autonomysim.utils.convs import to_quaternion
from autonomysim.utils.io import get_pfm_array, write_pfm, write_file


def main():
    # In settings.json first activate computer vision mode:
    # https://github.com/nervosys/AutonomySim/blob/master/docs/apis_image.md#computer-vision-mode

    SetupPath()

    pp = pprint.PrettyPrinter(indent=4)

    client = VehicleClient()

    wait_key("Press any key to get camera parameters")

    for camera_id in range(2):
        camera_info = client.simGetCameraInfo(str(camera_id))
        print("CameraInfo %d: %s" % (camera_id, pp.pprint(camera_info)))

    wait_key("Press any key to get images")
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
            Pose(Vector3r(x, 0, -2), to_quaternion(0, 0, 0)),
            True,
        )
        time.sleep(0.1)

        responses = client.simGetImages(
            [
                ImageRequest("0", ImageType.Scene),
                ImageRequest("1", ImageType.Scene),
                ImageRequest("2", ImageType.Scene),
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
                write_pfm(
                    os.path.normpath(
                        os.path.join(tmp_dir, str(x) + "_" + str(i) + ".pfm")
                    ),
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
        Pose(Vector3r(0, 0, 0), to_quaternion(0, 0, 0)),
        True,
    )


if __name__ == "__main__":
    main()
