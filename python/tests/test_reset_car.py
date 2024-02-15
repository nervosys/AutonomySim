import time

from autonomysim.clients import CarClient, CarControls
from autonomysim.utils import SetupPath


def main():
    SetupPath()

    # connect to the AutonomySim simulator
    client = CarClient()
    client.confirmConnection()
    client.enableApiControl(True)
    client.armDisarm(True)
    car_controls = CarControls()

    # go forward
    car_controls.throttle = 1
    car_controls.steering = 1
    client.setCarControls(car_controls)
    print("Go Forward")
    time.sleep(5)  # let car drive a bit

    print("reset")
    client.reset()
    time.sleep(5)  # let car drive a bit

    client.setCarControls(car_controls)
    print("Go Forward")
    time.sleep(5)  # let car drive a bit


if __name__ == "__main__":
    main()
