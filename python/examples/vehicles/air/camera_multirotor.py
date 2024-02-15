import sys
import time
import cv2

from autonomysim.types import ImageType
from autonomysim.utils import SetupPath
from autonomysim.utils.convs import string_to_uint8_array
from autonomysim.clients import MultirotorClient


def usage():
    print("Usage: python camera.py [depth|segmentation|scene]")


def main():
    """"""
    # In settings.json first activate computer vision mode:
    # https://github.com/nervosys/AutonomySim/blob/master/docs/apis_image.md#computer-vision-mode
    SetupPath()

    cameraType = "depth"

    for arg in sys.argv[1:]:
        cameraType = arg.lower()

    cameraTypeMap = {
        "depth": ImageType.DepthVis,
        "segmentation": ImageType.Segmentation,
        "seg": ImageType.Segmentation,
        "scene": ImageType.Scene,
        "disparity": ImageType.DisparityNormalized,
        "normals": ImageType.SurfaceNormals,
    }

    if cameraType not in cameraTypeMap:
        usage()
        sys.exit(0)

    print(cameraTypeMap[cameraType])

    client = MultirotorClient()

    print("Connected: now while this script is running, you can open another")
    print("console and run a script that flies the drone and this script will")
    print("show the depth view while the drone is flying.")

    fontFace = cv2.FONT_HERSHEY_SIMPLEX
    fontScale = 0.5
    thickness = 2
    textSize, baseline = cv2.getTextSize("FPS", fontFace, fontScale, thickness)
    print(textSize)
    textOrg = (10, 10 + textSize[1])
    frameCount = 0
    startTime = time.time()
    fps = 0

    while True:
        # because this method returns std::vector<uint8>, msgpack decides to encode it as a string unfortunately.
        rawImage = client.simGetImage("0", cameraTypeMap[cameraType])
        if rawImage is None:
            print(
                "Camera is not returning image, please check AutonomySim for error messages"
            )
            sys.exit(0)
        else:
            png = cv2.imdecode(string_to_uint8_array(rawImage), cv2.IMREAD_UNCHANGED)
            cv2.putText(
                png,
                "FPS " + str(fps),
                textOrg,
                fontFace,
                fontScale,
                (255, 0, 255),
                thickness,
            )
            cv2.imshow("Depth", png)

        frameCount = frameCount + 1
        endTime = time.time()
        diff = endTime - startTime
        if diff > 1:
            fps = frameCount
            frameCount = 0
            startTime = endTime

        key = cv2.waitKey(1) & 0xFF
        if key == 27 or key == ord("q") or key == ord("x"):
            break


if __name__ == "__main__":
    main()
