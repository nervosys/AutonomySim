from autonomysim.clients import VehicleClient
from autonomysim.types import Vector3r

import time


def main():
    client = VehicleClient()
    client.confirmConnection()

    # Access an existing light in the world
    lights = client.simListSceneObjects("PointLight.*")
    pose = client.simGetObjectPose(lights[0])
    scale = Vector3r(1, 1, 1)

    # Destroy the light
    client.simDestroyObject(lights[0])
    time.sleep(1)

    # Create a new light at the same pose
    new_light_name = client.simSpawnObject(
        "PointLight", "PointLightBP", pose, scale, False, True
    )
    time.sleep(1)

    # Change the light's intensity
    for i in range(20):
        client.simSetLightIntensity(new_light_name, i * 100)
        time.sleep(0.5)


if __name__ == "__main__":
    main()
