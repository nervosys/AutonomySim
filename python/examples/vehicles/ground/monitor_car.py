import time
import cv2

from autonomysim.clients import CarClient, CarControls
from autonomysim.utils import SetupPath


def main():
    SetupPath()

    # connect to the AutonomySim simulator
    client = CarClient()
    client.confirmConnection()
    car_controls = CarControls()

    start = time.time()

    print("Time,Speed,Gear,PX,PY,PZ,OW,OX,OY,OZ")

    # monitor car state while you drive it manually.
    while (cv2.waitKey(1) & 0xFF) == 0xFF:
        # get state of the car
        car_state = client.getCarState()
        pos = car_state.kinematics_estimated.position
        orientation = car_state.kinematics_estimated.orientation
        milliseconds = (time.time() - start) * 1000
        print(
            "%s,%d,%d,%f,%f,%f,%f,%f,%f,%f"
            % (
                milliseconds,
                car_state.speed,
                car_state.gear,
                pos.x_val,
                pos.y_val,
                pos.z_val,
                orientation.w_val,
                orientation.x_val,
                orientation.y_val,
                orientation.z_val,
            )
        )
        time.sleep(0.1)


if __name__ == "__main__":
    main()