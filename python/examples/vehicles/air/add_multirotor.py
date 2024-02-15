import tempfile
import os
import pprint

from autonomysim.types import Pose, Vector3r, ImageRequest, ImageType
from autonomysim.utils import SetupPath
from autonomysim.utils.convs import to_quaternion
from autonomysim.utils.io import write_file, write_pfm, get_pfm_array
from autonomysim.clients import MultirotorClient


def add_drone():
    SetupPath()

    # connect to the AutonomySim simulator
    client = MultirotorClient()
    client.confirmConnection()

    # add new vehicle
    vehicle_name = "Drone2"
    pose = Pose(Vector3r(0, 0, 0), to_quaternion(0, 0, 0))

    client.simAddVehicle(vehicle_name, "simpleflight", pose)
    client.enableApiControl(True, vehicle_name)
    client.armDisarm(True, vehicle_name)
    client.takeoffAsync(10.0, vehicle_name)

    requests = [
        ImageRequest("0", ImageType.DepthVis),  # scene depth image
        ImageRequest(
            "1", ImageType.DepthPerspective, True
        ),  # depth in perspective projection
        ImageRequest("1", ImageType.Scene),  # scene RGB image in png format
        ImageRequest(
            "1", ImageType.Scene, False, False
        ),  # scene uncompressed RGBA image array
    ]

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
            write_pfm(os.path.normpath(filename + ".pfm"), get_pfm_array(response))
        else:
            print(
                "Type %d, size %d, pos %s"
                % (
                    response.image_type,
                    len(response.image_data_uint8),
                    pprint.pformat(response.camera_position),
                )
            )
            write_file(os.path.normpath(filename + ".png"), response.image_data_uint8)
