# Convert Depth Images to Point Clouds

Moved here from [there](https://github.com/nervosys/AutonomySim/wiki/Point-Clouds).

A Python script [point_cloud.py](https://github.com/nervosys/AutonomySim/blob/master/PythonClient/multirotor/point_cloud.py) shows how to convert depth images returned from `AutonomySim` into point clouds.

The following depth image was captured using the `Modular Neighborhood` environment:

![depth](media/images/depth.png)

And with the appropriate projection matrix, the OpenCV `reprojectImageTo3D` function can turn this into a point cloud. The following is the result, which is also available here: [https://skfb.ly/68r7y](https://skfb.ly/68r7y).

![depth](media/images/point_cloud.png)

[SketchFab](https://sketchfab.com) can upload the resulting file `cloud.asc` and render it for you.

!!! warning
    You may notice the scene is reflected on the y-axis, so we may have a sign wrong in the projection matrix.
