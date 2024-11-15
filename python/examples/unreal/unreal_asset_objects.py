import time
import random

from autonomysim.clients import VehicleClient
from autonomysim.types import Vector3r, Pose
from autonomysim.utils import SetupPath


def main():
    SetupPath()

    client = VehicleClient()
    client.confirmConnection()

    assets = client.simListAssets()
    print(f"Assets: {assets}")

    scale = Vector3r(1.0, 1.0, 1.0)

    # asset_name = random.choice(assets)
    asset_name = "1M_Cube_Chamfer"

    desired_name = f"{asset_name}_spawn_{random.randint(0, 100)}"
    pose = Pose(position_val=Vector3r(5.0, 0.0, 0.0))

    obj_name = client.simSpawnObject(desired_name, asset_name, pose, scale, True)

    print(
        f"Created object {obj_name} from asset {asset_name} "
        f"at pose {pose}, scale {scale}"
    )

    all_objects = client.simListSceneObjects()
    if obj_name not in all_objects:
        print(f"Object {obj_name} not present!")

    time.sleep(10.0)

    print(f"Destroying {obj_name}")
    client.simDestroyObject(obj_name)


if __name__ == "__main__":
    main()
