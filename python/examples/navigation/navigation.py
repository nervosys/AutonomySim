import numpy as np
# import pprint

from autonomysim.clients import VehicleClient
from autonomysim.types import Pose, Vector3r, radians, ImageRequest, ImageType, wait_key
from autonomysim.utils import SetupPath
from autonomysim.utils.convs import to_quaternion
from autonomysim.ai.vision.navigation import AvoidLeft, moveUAV


def main():
    SetupPath()
    # In settings.json first activate computer vision mode:
    # https://github.com/nervosys/AutonomySim/blob/master/docs/apis_image.md#computer-vision-mode

    # pp = pprint.PrettyPrinter(indent=4)

    client = VehicleClient()
    client.confirmConnection()

    # tmp_dir = os.path.join(tempfile.gettempdir(), "autonomysim_drone")
    # print ("Saving images to %s" % tmp_dir)
    # autonomysim.wait_key('Press any key to start')

    # Define start position, goal and size of UAV
    pos = [0, 5, -1]  # start position x,y,z
    goal = [120, 0]  # x,y
    uav_size = [0.29 * 3, 0.98 * 2]  # height:0.29 x width:0.98 - allow some tolerance

    # Define parameters and thresholds
    hfov = radians(90)
    coll_thres = 5
    yaw = 0
    limit_yaw = 5
    step = 0.1

    responses = client.simGetImages([ImageRequest("1", ImageType.DepthPlanar, True)])
    response = responses[0]

    # initial position
    moveUAV(client, pos, yaw)

    # predictControl = AvoidLeftIgonreGoal(hfov, coll_thres, yaw, limit_yaw, step)
    predictControl = AvoidLeft(hfov, coll_thres, yaw, limit_yaw, step)

    for z in range(10000):
        # do a few times
        # time.sleep(1)

        # get response
        responses = client.simGetImages(
            [ImageRequest("1", ImageType.DepthPlanar, True)]
        )
        response = responses[0]

        # get numpy array
        img1d = response.image_data_float

        # reshape array to 2D array H X W
        img2d = np.reshape(img1d, (response.height, response.width))

        [pos, yaw, target_dist] = predictControl.get_next_vec(
            img2d, uav_size, goal, pos
        )
        moveUAV(client, pos, yaw)

        if target_dist < 1:
            print("Target reached.")
            wait_key("Press any key to continue...")
            break

        # write to png
        # imsave(os.path.normpath(os.path.join(tmp_dir, "depth_" + str(z) + '.png')), generate_depth_viz(img2d,5))

        # pose = client.simGetPose()
        # pp.pprint(pose)
        # time.sleep(5)

    # currently reset() doesn't work in CV mode. Below is the workaround
    client.simSetVehiclePose(
        Pose(Vector3r(0, 0, 0), to_quaternion(0, 0, 0)),
        True,
    )
    return None


if __name__ == "__main__":
    main()
