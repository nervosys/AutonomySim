import sys
import math
import numpy as np
import cv2

from autonomysim.types import ImageType
from autonomysim.utils import wait_key, SetupPath
from autonomysim.clients import MultirotorClient


def usage():
    print("Usage: python point_cloud.py [cloud.txt]")


def savePointCloud(image, filename):
    color = (0, 255, 0)
    rgb = "%d %d %d" % color
    f = open(filename, "w")
    for x in range(image.shape[0]):
        for y in range(image.shape[1]):
            pt = image[x, y]
            if math.isinf(pt[0]) or math.isnan(pt[0]):
                # skip it
                None
            else:
                f.write("%f %f %f %s\n" % (pt[0], pt[1], pt[2] - 1, rgb))
    f.close()


def main():
    """Use OpenCV to create a point cloud from a depth image.

    File will be saved in PythonClient folder (i.e. same folder as script)
    point cloud ASCII format, use viewers like CloudCompare http://www.danielgm.net/cc/
    or see http://www.geonext.nl/wp-content/uploads/2014/05/Point-Cloud-Viewers.pdf

    WARNING: this is a work in progress!
    """
    SetupPath()

    outputFile = "cloud.asc"
    projectionMatrix = np.array(
        [
            [-0.501202762, 0.000000000, 0.000000000, 0.000000000],
            [0.000000000, -0.501202762, 0.000000000, 0.000000000],
            [0.000000000, 0.000000000, 10.00000000, 100.00000000],
            [0.000000000, 0.000000000, -10.0000000, 0.000000000],
        ]
    )

    for arg in sys.argv[1:]:
        "cloud.txt" == arg

    client = MultirotorClient()

    while True:
        rawImage = client.simGetImage("0", ImageType.DepthPerspective)
        if rawImage is None:
            print(
                "Camera not returning images. Please check AutonomySim for error messages."
            )
            usage()
            wait_key("Press any key to exit")
            sys.exit(0)
        else:
            png = cv2.imdecode(np.frombuffer(rawImage, np.uint8), cv2.IMREAD_UNCHANGED)
            gray = cv2.cvtColor(png, cv2.COLOR_BGR2GRAY)
            Image3D = cv2.reprojectImageTo3D(gray, projectionMatrix)
            savePointCloud(Image3D, outputFile)
            print("saved " + outputFile)
            wait_key("Press any key to exit")
            sys.exit(0)

        key = cv2.waitKey(1) & 0xFF
        if key == 27 or key == ord("q") or key == ord("x"):
            break


if __name__ == "__main__":
    main()
