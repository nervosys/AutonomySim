from autonomysim.clients import VehicleClient
from autonomysim.utils import SetupPath
from autonomysim.utils.convs import to_eularian_angles


def main():
    # In settings.json first activate computer vision mode:
    # https://github.com/nervosys/AutonomySim/blob/master/docs/apis_image.md#computer-vision-mode
    SetupPath()

    client = VehicleClient()
    client.confirmConnection()

    pose = client.simGetVehiclePose()
    print(
        "x={}, y={}, z={}".format(
            pose.position.x_val, pose.position.y_val, pose.position.z_val
        )
    )

    angles = to_eularian_angles(client.simGetVehiclePose().orientation)
    print("pitch={}, roll={}, yaw={}".format(angles[0], angles[1], angles[2]))

    pose.position.x_val = pose.position.x_val + 1
    client.simSetVehiclePose(pose, True)


if __name__ == "__main__":
    main()
