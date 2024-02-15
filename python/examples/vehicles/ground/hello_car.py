import os
import time
import tempfile
import numpy as np
import cv2

from autonomysim.clients import CarClient, CarControls
from autonomysim.types import ImageRequest, ImageType
from autonomysim.utils import SetupPath
from autonomysim.utils.io import get_pfm_array, write_file, write_pfm


def main():
    SetupPath()

    # connect to the AutonomySim simulator
    client = CarClient()
    client.confirmConnection()
    client.enableApiControl(True)
    print("API Control enabled: %s" % client.isApiControlEnabled())
    car_controls = CarControls()

    tmp_dir = os.path.join(tempfile.gettempdir(), "autonomysim_car")
    print("Saving images to %s" % tmp_dir)
    try:
        os.makedirs(tmp_dir)
    except OSError:
        if not os.path.isdir(tmp_dir):
            raise

    for idx in range(3):
        # get state of the car
        car_state = client.getCarState()
        print("Speed %d, Gear %d" % (car_state.speed, car_state.gear))

        # go forward
        car_controls.throttle = 0.5
        car_controls.steering = 0
        client.setCarControls(car_controls)
        print("Go Forward")
        time.sleep(3)  # let car drive a bit

        # Go forward + steer right
        car_controls.throttle = 0.5
        car_controls.steering = 1
        client.setCarControls(car_controls)
        print("Go Forward, steer right")
        time.sleep(3)  # let car drive a bit

        # go reverse
        car_controls.throttle = -0.5
        car_controls.is_manual_gear = True
        car_controls.manual_gear = -1
        car_controls.steering = 0
        client.setCarControls(car_controls)
        print("Go reverse, steer right")
        time.sleep(3)  # let car drive a bit
        car_controls.is_manual_gear = False  # change back gear to auto
        car_controls.manual_gear = 0

        # apply brakes
        car_controls.brake = 1
        client.setCarControls(car_controls)
        print("Apply brakes")
        time.sleep(3)  # let car drive a bit
        car_controls.brake = 0  # remove brake

        # get camera images from the car
        responses = client.simGetImages(
            [
                ImageRequest("0", ImageType.DepthVis),  # depth visualization image
                ImageRequest(
                    "1", ImageType.DepthPerspective, True
                ),  # depth in perspective projection
                ImageRequest("1", ImageType.Scene),  # scene vision image in png format
                ImageRequest("1", ImageType.Scene, False, False),
            ]
        )  # scene vision image in uncompressed RGB array
        print("Retrieved images: %d" % len(responses))

        for response_idx, response in enumerate(responses):
            filename = os.path.join(
                tmp_dir, f"{idx}_{response.image_type}_{response_idx}"
            )

            if response.pixels_as_float:
                print(
                    "Type %d, size %d"
                    % (response.image_type, len(response.image_data_float))
                )
                write_pfm(os.path.normpath(filename + ".pfm"), get_pfm_array(response))
            elif response.compress:  # png format
                print(
                    "Type %d, size %d"
                    % (response.image_type, len(response.image_data_uint8))
                )
                write_file(
                    os.path.normpath(filename + ".png"), response.image_data_uint8
                )
            else:  # uncompressed array
                print(
                    "Type %d, size %d"
                    % (response.image_type, len(response.image_data_uint8))
                )
                img1d = np.fromstring(
                    response.image_data_uint8, dtype=np.uint8
                )  # get numpy array
                img_rgb = img1d.reshape(
                    response.height, response.width, 3
                )  # reshape array to 3 channel image array H X W X 3
                cv2.imwrite(
                    os.path.normpath(filename + ".png"), img_rgb
                )  # write to png

    # restore to original state
    client.reset()

    client.enableApiControl(False)


if __name__ == "__main__":
    main()
