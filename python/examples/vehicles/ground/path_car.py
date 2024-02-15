import numpy as np

from autonomysim.clients import CarClient, CarControls
from autonomysim.utils import SetupPath


def main():
    SetupPath()

    # connect to the AutonomySim simulator
    client = CarClient()
    client.confirmConnection()
    client.enableApiControl(True)
    car_controls = CarControls()

    car_controls.steering = 0
    car_controls.throttle = 0
    car_controls.brake = 0

    image_buf = np.zeros((1, 144, 256, 3))
    state_buf = np.zeros((1, 4))

    while True:
        car_state = client.getCarState()

        print("car speed: {0}".format(car_state.speed))

        if car_state.speed < 20:
            car_controls.throttle = 1.0
        else:
            car_controls.throttle = 0.0

        # state_buf[0] = np.array([car_controls.steering, car_controls.throttle, car_controls.brake, car_state.speed])
        # model_output = model.predict([image_buf, state_buf])
        # car_controls.steering = float(model_output[0][0])
        car_controls.steering = 0

        print(
            "Sending steering = {0}, throttle = {1}".format(
                car_controls.steering, car_controls.throttle
            )
        )

        client.setCarControls(car_controls)
