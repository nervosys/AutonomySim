import time
import sys
import os
import random
import glob
import numpy
import cv2

from autonomysim.types import *
from autonomysim.utils.convs import *


def radiance(absoluteTemperature, emissivity, dx=0.01, response=None):
    """
    title::
        radiance

    description::
        Calculates radiance and integrated radiance over a bandpass of 8 to 14
        microns, given temperature and emissivity, using Planck's Law.

    inputs::
        absoluteTemperature
            temperture of object in [K]

            either a single temperature or a numpy
            array of temperatures, of shape (temperatures.shape[0], 1)
        emissivity
            average emissivity (number between 0 and 1 representing the
            efficiency with which it emits radiation; if 1, it is an ideal
            blackbody) of object over the bandpass

            either a single emissivity or a numpy array of emissivities, of
            shape (emissivities.shape[0], 1)
        dx
            discrete spacing between the wavelengths for evaluation of
            radiance and integration [default is 0.1]
        response
            optional response of the camera over the bandpass of 8 to 14
            microns [default is None, for no response provided]

    returns::
        radiance
            discrete spectrum of radiance over bandpass
        integratedRadiance
            integration of radiance spectrum over bandpass (to simulate
            the readout from a sensor)

    author::
        Elizabeth Bondi
    """
    wavelength = numpy.arange(8, 14, dx)
    c1 = 1.19104e8  # (2 * 6.62607*10^-34 [Js] *
    # (2.99792458 * 10^14 [micron/s])^2 * 10^12 to convert
    # denominator from microns^3 to microns * m^2)
    c2 = 1.43879e4  # (hc/k) [micron * K]
    if response is not None:
        radiance = (
            response
            * emissivity
            * (
                c1
                / (
                    (wavelength**5)
                    * (numpy.exp(c2 / (wavelength * absoluteTemperature)) - 1)
                )
            )
        )
    else:
        radiance = emissivity * (
            c1
            / (
                (wavelength**5)
                * (numpy.exp(c2 / (wavelength * absoluteTemperature)) - 1)
            )
        )
    if absoluteTemperature.ndim > 1:
        return radiance, numpy.trapz(radiance, dx=dx, axis=1)
    else:
        return radiance, numpy.trapz(radiance, dx=dx)


def get_new_temp_emiss_from_radiance(tempEmissivity, response):
    """
    title::
        get_new_temp_emiss_from_radiance

    description::
        Transform tempEmissivity from [objectName, temperature, emissivity]
        to [objectName, "radiance"] using radiance calculation above.

    input::
        tempEmissivity
            numpy array containing the temperature and emissivity of each
            object (e.g., each row has: [objectName, temperature, emissivity])
        response
            camera response (same input as radiance, set to None if lacking
            this information)

    returns::
        tempEmissivityNew
            tempEmissivity, now with [objectName, "radiance"]; note that
            integrated radiance (L) is divided by the maximum and multiplied
            by 255 in order to simulate an 8 bit digital count observed by the
            thermal sensor, since radiance and digital count are linearly
            related, so it's [objectName, simulated thermal digital count]

    author::
        Elizabeth Bondi
    """
    numObjects = tempEmissivity.shape[0]

    L = radiance(
        tempEmissivity[:, 1].reshape((-1, 1)).astype(numpy.float64),
        tempEmissivity[:, 2].reshape((-1, 1)).astype(numpy.float64),
        response=response,
    )[1].flatten()
    L = ((L / L.max()) * 255).astype(numpy.uint8)

    tempEmissivityNew = numpy.hstack(
        (tempEmissivity[:, 0].reshape((numObjects, 1)), L.reshape((numObjects, 1)))
    )

    return tempEmissivityNew


def set_segmentation_ids(segIdDict, tempEmissivityNew, client):
    """
    title::
        set_segmentation_ids

    description::
        Set stencil IDs in environment so that stencil IDs correspond to
        simulated thermal digital counts (e.g., if elephant has a simulated
        digital count of 219, set stencil ID to 219).

    input::
        segIdDict
            dictionary mapping environment object names to the object names in
            the first column of tempEmissivityNew
        tempEmissivityNew
            numpy array containing object names and corresponding simulated
            thermal digital count
        client
            connection to autonomysim (e.g., client = MultirotorClient() for UAV)

    author::
        Elizabeth Bondi
    """

    # First set everything to 0.
    success = client.simSetSegmentationObjectID("[\w]*", 0, True)
    if not success:
        print("There was a problem setting all segmentation object IDs to 0. ")
        sys.exit(1)

    # Next set all objects of interest provided to corresponding object IDs
    # segIdDict values MUST match tempEmissivityNew labels.
    for key in segIdDict:
        objectID = int(
            tempEmissivityNew[numpy.where(tempEmissivityNew == segIdDict[key])[0], 1][0]
        )

        success = client.simSetSegmentationObjectID(
            "[\w]*" + key + "[\w]*", objectID, True
        )
        if not success:
            print(
                "There was a problem setting {0} segmentation object ID to {1!s}, or no {0} was found.".format(
                    key, objectID
                )
            )

    time.sleep(0.1)


def rotation_matrix_from_angles(pry):
    pitch = pry[0]
    roll = pry[1]
    yaw = pry[2]
    sy = numpy.sin(yaw)
    cy = numpy.cos(yaw)
    sp = numpy.sin(pitch)
    cp = numpy.cos(pitch)
    sr = numpy.sin(roll)
    cr = numpy.cos(roll)

    Rx = numpy.array([[1, 0, 0], [0, cr, -sr], [0, sr, cr]])

    Ry = numpy.array([[cp, 0, sp], [0, 1, 0], [-sp, 0, cp]])

    Rz = numpy.array([[cy, -sy, 0], [sy, cy, 0], [0, 0, 1]])

    # Roll is applied first, then pitch, then yaw.
    RyRx = numpy.matmul(Ry, Rx)
    return numpy.matmul(Rz, RyRx)


def project_3d_point_to_screen(
    subjectXYZ, camXYZ, camQuaternion, camProjMatrix4x4, imageWidthHeight
):
    # Turn the camera position into a column vector.
    camPosition = numpy.transpose([camXYZ])

    # Convert the camera's quaternion rotation to yaw, pitch, roll angles.
    pitchRollYaw = utils.to_eularian_angles(camQuaternion)

    # Create a rotation matrix from camera pitch, roll, and yaw angles.
    camRotation = rotation_matrix_from_angles(pitchRollYaw)

    # Change coordinates to get subjectXYZ in the camera's local coordinate system.
    XYZW = numpy.transpose([subjectXYZ])
    XYZW = numpy.add(XYZW, -camPosition)
    print("XYZW: " + str(XYZW))
    XYZW = numpy.matmul(numpy.transpose(camRotation), XYZW)
    print("XYZW derot: " + str(XYZW))

    # Recreate the perspective projection of the camera.
    XYZW = numpy.concatenate([XYZW, [[1]]])
    XYZW = numpy.matmul(camProjMatrix4x4, XYZW)
    XYZW = XYZW / XYZW[3]

    # Move origin to the upper-left corner of the screen and multiply by size to get pixel values. Note that screen is in y,-z plane.
    normX = (1 - XYZW[0]) / 2
    normY = (1 + XYZW[1]) / 2

    return numpy.array(
        [imageWidthHeight[0] * normX, imageWidthHeight[1] * normY]
    ).reshape(
        2,
    )


def get_image(x, y, z, pitch, roll, yaw, client):
    """
    title::
        get_image

    description::
        Capture images (as numpy arrays) from a certain position.

    inputs::
        x
            x position in meters
        y
            y position in meters
        z
            altitude in meters; remember NED, so should be negative to be
            above ground
        pitch
            angle (in radians); in computer vision mode, this is camera angle
        roll
            angle (in radians)
        yaw
            angle (in radians)
        client
            connection to autonomysim (e.g., client = MultirotorClient() for UAV)

    returns::
        position
            autonomysim position vector (access values with x_val, y_val, z_val)
        angle
            autonomysim quaternion ("angles")
        im
            segmentation or IR image, depending upon palette in use (3 bands)
        imScene
            scene image (3 bands)

    author::
        Elizabeth Bondi
        Shital Shah
    """

    # Set pose and sleep after to ensure the pose sticks before capturing image.
    client.simSetVehiclePose(
        Pose(Vector3r(x, y, z), to_quaternion(pitch, roll, yaw)), True
    )
    time.sleep(0.1)

    # Capture segmentation (IR) and scene images.
    responses = client.simGetImages(
        [
            ImageRequest("0", ImageType.Infrared, False, False),
            ImageRequest("0", ImageType.Scene, False, False),
            ImageRequest("0", ImageType.Segmentation, False, False),
        ]
    )

    # Change images into numpy arrays.
    img1d = numpy.fromstring(responses[0].image_data_uint8, dtype=numpy.uint8)
    im = img1d.reshape(responses[0].height, responses[0].width, 4)

    img1dscene = numpy.fromstring(responses[1].image_data_uint8, dtype=numpy.uint8)
    imScene = img1dscene.reshape(responses[1].height, responses[1].width, 4)

    return (
        Vector3r(x, y, z),
        to_quaternion(pitch, roll, yaw),
        im[:, :, :3],
        imScene[:, :, :3],
    )  # get rid of alpha channel
