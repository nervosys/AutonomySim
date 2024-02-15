import os

os.environ["TF_CPP_MIN_LOG_LEVEL"] = "2"

import time
import numpy as np
from keras.models import load_model

from autonomysim.clients import CarClient, CarControls
from autonomysim.types import ImageRequest, ImageType


# Trained model path
MODEL_PATH = "./models/example_model.h5"


class CarAgent:
    model = None

    def __init__(self) -> None:
        pass

    def get_image(self, client):
        """
        Get image from AutonomySim client
        """

        image_response = client.simGetImages(
            [ImageRequest("0", ImageType.Scene, False, False)]
        )[0]
        image1d = np.fromstring(image_response.image_data_uint8, dtype=np.uint8)
        image_rgb = image1d.reshape(image_response.height, image_response.width, 3)
        return image_rgb[78:144, 27:227, 0:2].astype(float)

    def load(self, model_path=MODEL_PATH):
        """
        Load the model.
        """
        self.model = load_model(model_path)

    def run(self):
        """
        Run the model.
        """

        # Connect to autonomysim
        client = CarClient()
        client.confirmConnection()
        client.enableApiControl(True)
        car_controls = CarControls()

        # Start driving
        car_controls.steering = 0
        car_controls.throttle = 0
        car_controls.brake = 0
        client.setCarControls(car_controls)

        # Initialize image buffer
        image_buf = np.zeros((1, 66, 200, 3))

        while True:
            # Update throttle value according to steering angle
            if abs(car_controls.steering) <= 1.0:
                car_controls.throttle = 0.8 - (0.4 * abs(car_controls.steering))
            else:
                car_controls.throttle = 0.4

            image_buf[0] = self.get_image(client)
            image_buf[0] /= 255  # normalization

            start_time = time.time()

            # Prediction
            model_output = self.model.predict([image_buf])

            end_time = time.time()
            received_output = model_output[0][0]

            # Rescale prediction to [-1,1] and factor by 0.82 for drive smoothness
            car_controls.steering = round(
                (0.82 * (float((model_output[0][0] * 2.0) - 1))), 2
            )

            # Print progress
            print(
                "Sending steering = {0}, throttle = {1}, prediction time = {2}".format(
                    received_output, car_controls.throttle, str(end_time - start_time)
                )
            )

            # Update next car state
            client.setCarControls(car_controls)

            # Wait a bit between iterations
            time.sleep(0.05)

        client.enableApiControl(False)
        return None
