from datetime import datetime

from autonomysim.types import ImageRequest, ImageType
from autonomysim.clients import VehicleClient


def main():
    """
    Simple script with settings to create a high-resolution camera, and fetching it.

    {
        "SettingsVersion": 1.2,
        "SimMode": "Multirotor",
        "Vehicles" : {
            "Drone1" : {
                "VehicleType" : "SimpleFlight",
                "AutoCreate" : true,
                "Cameras" : {
                    "high_res": {
                        "CaptureSettings" : [
                            {
                                "ImageType" : 0,
                                "Width" : 4320,
                                "Height" : 2160
                            }
                        ],
                        "X": 0.50, "Y": 0.00, "Z": 0.10,
                        "Pitch": 0.0, "Roll": 0.0, "Yaw": 0.0
                    },
                    "low_res": {
                        "CaptureSettings" : [
                            {
                                "ImageType" : 0,
                                "Width" : 256,
                                "Height" : 144
                            }
                        ],
                        "X": 0.50, "Y": 0.00, "Z": 0.10,
                        "Pitch": 0.0, "Roll": 0.0, "Yaw": 0.0
                    }
                }
            }
        }
    }
    """

    client = VehicleClient()
    client.confirmConnection()
    framecounter = 1

    prevtimestamp = datetime.now()

    while framecounter <= 500:
        if framecounter % 150 == 0:
            client.simGetImages(
                [ImageRequest("high_res", ImageType.Scene, False, False)]
            )
            print("High resolution image captured.")

        if framecounter % 30 == 0:
            now = datetime.now()
            print(f"Time spent for 30 frames: {now - prevtimestamp}")
            prevtimestamp = now

        client.simGetImages([ImageRequest("low_res", ImageType.Scene, False, False)])
        framecounter += 1


if __name__ == "__main__":
    main()
