import os
import tempfile

from autonomysim.clients import VehicleClient
from autonomysim.types import Pose, Vector3r, ImageRequest, ImageType, wait_key
from autonomysim.utils import SetupPath
from python.autonomysim.utils.io import get_pfm_array, write_pfm, write_file
from autonomysim.utils.convs import to_quaternion


# Just change the below to test different cameras easily!
CAM_NAME = "fixed1"
IS_EXTERNAL_CAM = True


def save_images(responses, prefix=""):
    tmp_dir = os.path.join(tempfile.gettempdir(), "autonomysim_cv_mode")
    for i, response in enumerate(responses):
        filename = os.path.join(tmp_dir, prefix + "_" + str(i))
        if response.pixels_as_float:
            print(
                f"Type {response.image_type}, size {len(response.image_data_float)}, pos {response.camera_position}"
            )
            write_pfm(os.path.normpath(filename + ".pfm"), get_pfm_array(response))
        else:
            print(
                f"Type {response.image_type}, size {len(response.image_data_uint8)}, pos {response.camera_position}"
            )
            write_file(os.path.normpath(filename + ".png"), response.image_data_uint8)


def main():
    """
    A simple script to test all the camera APIs. Change the camera name and whether it's an external camera

    Example Settings for external camera -

    {
        "SettingsVersion": 1.2,
        "SimMode": "Car",
        "ExternalCameras": {
            "fixed1": {
                "X": 0, "Y": 0, "Z": -5,
                "Pitch": -90, "Roll": 0, "Yaw": 0
            }
        }
    }
    """
    SetupPath()

    client = VehicleClient()
    client.confirmConnection()

    tmp_dir = os.path.join(tempfile.gettempdir(), "autonomysim_cv_mode")
    print("Saving images to %s" % tmp_dir)
    try:
        os.makedirs(tmp_dir)
    except OSError:
        if not os.path.isdir(tmp_dir):
            raise

    print(f"Camera: {CAM_NAME}, External = {IS_EXTERNAL_CAM}")

    # Test Camera info
    cam_info = client.simGetCameraInfo(CAM_NAME, external=IS_EXTERNAL_CAM)
    print(cam_info)

    # Test Image APIs
    wait_key("Press any key to get images")

    requests = [
        ImageRequest(CAM_NAME, ImageType.Scene),
        ImageRequest(CAM_NAME, ImageType.DepthPlanar),
        ImageRequest(CAM_NAME, ImageType.DepthVis),
        ImageRequest(CAM_NAME, ImageType.Segmentation),
        ImageRequest(CAM_NAME, ImageType.SurfaceNormals),
    ]

    responses = client.simGetImages(requests, external=IS_EXTERNAL_CAM)
    save_images(responses, "old_fov")

    # Test FoV API
    wait_key("Press any key to change FoV and get images")

    client.simSetCameraFov(CAM_NAME, 120, external=IS_EXTERNAL_CAM)

    responses = client.simGetImages(requests, external=IS_EXTERNAL_CAM)
    save_images(responses, "new_fov")

    new_cam_info = client.simGetCameraInfo(CAM_NAME, external=IS_EXTERNAL_CAM)
    print(f"Old FOV: {cam_info.fov}, New FOV: {new_cam_info.fov}")

    # Test Pose APIs
    new_pose = Pose(Vector3r(-10, -5, -5), to_quaternion(0.1, 0, 0.1))
    client.simSetCameraPose(CAM_NAME, new_pose, external=IS_EXTERNAL_CAM)

    responses = client.simGetImages(requests, external=IS_EXTERNAL_CAM)
    save_images(responses, "new_pose")

    new_cam_info = client.simGetCameraInfo(CAM_NAME, external=IS_EXTERNAL_CAM)
    print(f"Old Pose: {cam_info.pose}, New Pose: {new_cam_info.pose}")

    # Test Distortion params APIs
    dist_params = client.simGetDistortionParams(CAM_NAME, external=IS_EXTERNAL_CAM)
    print(f"Distortion Params: {dist_params}")

    new_params_dict = {"K1": 0.1, "K2": 0.01, "K3": 0.0, "P1": 0.0, "P2": 0.0}
    print(f"Setting distortion params as {new_params_dict}")
    client.simSetDistortionParams(CAM_NAME, new_params_dict, external=IS_EXTERNAL_CAM)

    dist_params = client.simGetDistortionParams(CAM_NAME, external=IS_EXTERNAL_CAM)
    print(f"Updated Distortion Params: {dist_params}")
