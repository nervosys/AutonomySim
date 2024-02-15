import time
import numpy as np

from autonomysim.clients import CarClient, CarControls
from autonomysim.types import ImageRequest, ImageType


def main():
    # connect to the AutonomySim simulator
    client = CarClient()
    client.confirmConnection()
    client.enableApiControl(True)
    car_controls = CarControls()

    # set the controls for car
    car_controls.throttle = -0.5
    car_controls.is_manual_gear = True
    car_controls.manual_gear = -1
    client.setCarControls(car_controls)

    # let car drive a bit
    time.sleep(10)

    client.simPause(True)
    car_position1 = client.getCarState().kinematics_estimated.position
    img_position1 = client.simGetImages([ImageRequest(0, ImageType.Scene)])[
        0
    ].camera_position
    print(f"Before pause position: {car_position1}")
    print(
        f"Before pause diff: {car_position1.x_val - img_position1.x_val}, {car_position1.y_val - img_position1.y_val}, {car_position1.z_val - img_position1.z_val}"
    )

    time.sleep(10)

    car_position2 = client.getCarState().kinematics_estimated.position
    img_position2 = client.simGetImages([ImageRequest(0, ImageType.Scene)])[
        0
    ].camera_position
    print(f"After pause position: {car_position2}")
    print(
        f"After pause diff: {car_position2.x_val - img_position2.x_val}, {car_position2.y_val - img_position2.y_val}, {car_position2.z_val - img_position2.z_val}"
    )
    client.simPause(False)


if __name__ == "__main__":
    main()
