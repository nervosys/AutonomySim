from autonomysim.clients import MultirotorClient
from python.autonomysim.sensors.thermal_camera import (
    get_image,
    project_3d_point_to_screen,
)

import time
import numpy
import cv2


def main(
    client,
    objectList,
    pitch=numpy.radians(270),  # image straight down
    roll=0,
    yaw=0,
    z=-122,
    writeIR=True,
    writeScene=False,
    irFolder="",
    sceneFolder="",
):
    """
    title::
        main

    description::
        Follow objects of interest and record images while following.

    inputs::
        client
            connection to autonomysim (e.g., client = MultirotorClient() for UAV)
        objectList
            list of tag names within the autonomysim environment, corresponding to
            objects to follow (add tags by clicking on object, going to
            Details, Actor, and Tags, then add component)
        pitch
            angle (in radians); in computer vision mode, this is camera angle
        roll
            angle (in radians)
        yaw
            angle (in radians)
        z
            altitude in meters; remember NED, so should be negative to be
            above ground
        write
            if True, will write out the images
        folder
            path to a particular folder that should be used (then within that
            folder, expected folders are ir and scene)

    author::
        Elizabeth Bondi
    """
    i = 0
    for o in objectList:
        startTime = time.time()
        elapsedTime = 0
        pose = client.simGetObjectPose(o)
        # Capture images for a certain amount of time in seconds (half hour now)
        while elapsedTime < 1800:
            # Capture image - pose.position x_val access may change w/ autonomysim
            # version (pose.position.x_val new, pose.position[b'x_val'] old)
            vector, angle, ir, scene = get_image(
                pose.position.x_val, pose.position.y_val, z, pitch, roll, yaw, client
            )

            # Convert color scene image to BGR for write out with cv2.
            r, g, b = cv2.split(scene)
            scene = cv2.merge((b, g, r))

            if writeIR:
                cv2.imwrite(irFolder + "ir_" + str(i).zfill(5) + ".png", ir)
            if writeScene:
                cv2.imwrite(sceneFolder + "scene_" + str(i).zfill(5) + ".png", scene)

            i += 1
            elapsedTime = time.time() - startTime
            pose = client.simGetObjectPose(o)
            camInfo = client.simGetCameraInfo("0")
            object_xy_in_pic = project_3d_point_to_screen(
                [pose.position.x_val, pose.position.y_val, pose.position.z_val],
                [
                    camInfo.pose.position.x_val,
                    camInfo.pose.position.y_val,
                    camInfo.pose.position.z_val,
                ],
                camInfo.pose.orientation,
                camInfo.proj_mat.matrix,
                ir.shape[:2][::-1],
            )
            print("Object projected to pixel\n{!s}.".format(object_xy_in_pic))


if __name__ == "__main__":
    # Connect to autonomysim, UAV mode.
    client = MultirotorClient()
    client.confirmConnection()

    # Look for objects with names that match a regular expression.
    poacherList = client.simListSceneObjects(".*?Poacher.*?")
    elephantList = client.simListSceneObjects(".*?Elephant.*?")
    crocList = client.simListSceneObjects(".*?Croc.*?")
    hippoList = client.simListSceneObjects(".*?Hippo.*?")

    objectList = elephantList

    # Sample calls to main, varying camera angle and altitude.
    # straight down, 400ft
    main(client, objectList, irFolder=r"auto\winter\400ft\down")
    # straight down, 200ft
    main(client, objectList, z=-61, irFolder=r"auto\winter\200ft\down")
    # 45 degrees, 200ft -- note that often object won't be scene since position
    # is set exactly to object's
    main(
        client,
        objectList,
        z=-61,
        pitch=numpy.radians(315),
        irFolder=r"auto\winter\200ft\45",
    )
    # 45 degrees, 400ft -- note that often object won't be scene since position
    # is set exactly to object's
    main(client, objectList, pitch=numpy.radians(315), irFolder=r"auto\winter\400ft\45")
