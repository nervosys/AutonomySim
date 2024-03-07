# ROS Interface

`autonomysim_ros_pkgs`: a C++ `ROS` wrapper over the `AutonomySim` client library.

## Setup

The below steps are for Linux. If you're running `AutonomySim` on Windows, you can use Windows Subsystem for Linux (WSL) to run the ROS wrapper. See the instructions [below](#setting-up-the-build-environment-on-windows10-using-wsl1-or-wsl2). If you are unable or prefer not to install ROS and related tools on your host Linux machine, you can also try it using Docker. See the steps [below](#using-docker-for-ros).

* If your default `gcc` version is less than 8 (see `gcc --version` output)
  * Install `gcc` >= 8.0.0: `sudo apt-get install gcc-11 g++-11`
  * Verify installation by `gcc-11 --version`
* Ubuntu 16.04
    * Install [ROS kinetic](https://wiki.ros.org/kinetic/Installation/Ubuntu)
    * Install `tf2-sensor` and `mavros` packages: `sudo apt-get install ros-kinetic-tf2-sensor-msgs ros-kinetic-tf2-geometry-msgs ros-kinetic-mavros*`
* Ubuntu 18.04
    * Install [ROS melodic](https://wiki.ros.org/melodic/Installation/Ubuntu)
    * Install `tf2-sensor` and `mavros` packages: `sudo apt-get install ros-melodic-tf2-sensor-msgs ros-melodic-tf2-geometry-msgs ros-melodic-mavros*`
* Ubuntu 20.04 | 22.04
    * Install [ROS noetic](https://wiki.ros.org/noetic/Installation/Ubuntu)
    * Install `tf2-sensor` and `mavros` packages: `sudo apt-get install ros-noetic-tf2-sensor-msgs ros-noetic-tf2-geometry-msgs ros-noetic-mavros*`

* Install [catkin_tools](https://catkin-tools.readthedocs.io/en/latest/installing.html)
    `sudo apt-get install python-catkin-tools` or `pip install catkin_tools`. If using Ubuntu 20.04 use `pip install "git+https://github.com/catkin/catkin_tools.git#egg=catkin_tools"`

## Build

Build `AutonomySim`:

```shell
git clone https://github.com/nervosys/AutonomySim.git;
cd AutonomySim;
./setup.sh;
./build.sh;
```

Ensure that you have setup the environment variables for ROS, as mentioned in the installation pages above. Add the `source` command to your `.bashrc` for convenience (replace `melodic` with specfic version name):

```shell
echo "source /opt/ros/melodic/setup.bash" >> ~/.bashrc
source ~/.bashrc
```

Build the ROS package:

```shell
cd ros;
catkin build;  # or catkin_make
```

If your default `gcc` version is less than 8 (see `gcc --version` output), compilation will fail. In that case, use `gcc-11` explicitly as follows:

```shell
catkin build -DCMAKE_C_COMPILER=/usr/bin/gcc-11 -DCMAKE_CXX_COMPILER=/usr/bin/g++-11
```

## Running `autonomysim_ros_pkgs`

```shell
source devel/setup.bash;
roslaunch autonomysim_ros_pkgs autonomysim_node.launch;
roslaunch autonomysim_ros_pkgs rviz.launch;
```

!!! note
  If you get an error running `roslaunch autonomysim_ros_pkgs autonomysim_node.launch`, run `catkin clean` and try again

## Using the AutonomySim ROS Wrapper

The ROS wrapper is composed of two ROS nodes:

1. A wrapper over the AutonomySim multirotor C++ client library.
2. A simple proportional derivative (PD) or [proportional integral derivative](https://www.cds.caltech.edu/~murray/courses/cds101/fa04/caltech/am04_ch8-3nov04.pdf) (PID) position controller.

Let's look at the ROS API for both nodes below.

### ROS Wrapper Node

#### Publishers

* `/autonomysim_node/origin_geo_point` [autonomysim_ros_pkgs/GPSYaw](https://github.com/nervosys/AutonomySim/tree/master/ros/src/autonomysim_ros_pkgs/msg/GPSYaw.msg)
GPS coordinates corresponding to global NED frame. This is set in the AutonomySim's [settings.json](https://nervosys.github.io/AutonomySim/settings/) file under the `OriginGeopoint` key.

* `/autonomysim_node/VEHICLE_NAME/global_gps` [sensor_msgs/NavSatFix](https://docs.ros.org/api/sensor_msgs/html/msg/NavSatFix.html)
This the current GPS coordinates of the drone in AutonomySim.

* `/autonomysim_node/VEHICLE_NAME/odom_local_ned` [nav_msgs/Odometry](https://docs.ros.org/api/nav_msgs/html/msg/Odometry.html)
Odometry in NED frame (default name: odom_local_ned, launch name and frame type are configurable) wrt take-off point.

* `/autonomysim_node/VEHICLE_NAME/CAMERA_NAME/IMAGE_TYPE/camera_info` [sensor_msgs/CameraInfo](https://docs.ros.org/api/sensor_msgs/html/msg/CameraInfo.html)

* `/autonomysim_node/VEHICLE_NAME/CAMERA_NAME/IMAGE_TYPE` [sensor_msgs/Image](https://docs.ros.org/api/sensor_msgs/html/msg/Image.html)
  RGB or float image depending on image type requested in settings.json.

* `/tf` [tf2_msgs/TFMessage](https://docs.ros.org/api/tf2_msgs/html/msg/TFMessage.html)

* `/autonomysim_node/VEHICLE_NAME/altimeter/SENSOR_NAME` [autonomysim_ros_pkgs/Altimeter](https://github.com/nervosys/AutonomySim/blob/master/ros/src/autonomysim_ros_pkgs/msg/Altimeter.msg)
This the current altimeter reading for altitude, pressure, and [QNH](https://en.wikipedia.org/wiki/QNH)

* `/autonomysim_node/VEHICLE_NAME/imu/SENSOR_NAME` [sensor_msgs::Imu](http://docs.ros.org/api/sensor_msgs/html/msg/Imu.html)
IMU sensor data

* `/autonomysim_node/VEHICLE_NAME/magnetometer/SENSOR_NAME` [sensor_msgs::MagneticField](http://docs.ros.org/api/sensor_msgs/html/msg/MagneticField.html)
  Meausrement of magnetic field vector/compass

* `/autonomysim_node/VEHICLE_NAME/distance/SENSOR_NAME` [sensor_msgs::Range](http://docs.ros.org/api/sensor_msgs/html/msg/Range.html)
  Meausrement of distance from an active ranger, such as infrared or IR

* `/autonomysim_node/VEHICLE_NAME/lidar/SENSOR_NAME` [sensor_msgs::PointCloud2](http://docs.ros.org/api/sensor_msgs/html/msg/PointCloud2.html)
  LIDAR pointcloud

#### Subscribers

* `/autonomysim_node/vel_cmd_body_frame` [autonomysim_ros_pkgs/VelCmd](https://github.com/nervosys/AutonomySim/tree/master/ros/src/autonomysim_ros_pkgs/msg/VelCmd.msg)
  Ignore `vehicle_name` field, leave it to blank. We will use `vehicle_name` in future for multiple drones.

* `/autonomysim_node/vel_cmd_world_frame` [autonomysim_ros_pkgs/VelCmd](https://github.com/nervosys/AutonomySim/tree/master/ros/src/autonomysim_ros_pkgs/msg/VelCmd.msg)
  Ignore `vehicle_name` field, leave it to blank. We will use `vehicle_name` in future for multiple drones.

* `/gimbal_angle_euler_cmd` [autonomysim_ros_pkgs/GimbalAngleEulerCmd](https://github.com/nervosys/AutonomySim/tree/master/ros/src/autonomysim_ros_pkgs/msg/GimbalAngleEulerCmd.msg)
  Gimbal set point in euler angles.

* `/gimbal_angle_quat_cmd` [autonomysim_ros_pkgs/GimbalAngleQuatCmd](https://github.com/nervosys/AutonomySim/tree/master/ros/src/autonomysim_ros_pkgs/msg/GimbalAngleQuatCmd.msg)
  Gimbal set point in quaternion.

* `/autonomysim_node/VEHICLE_NAME/car_cmd` [autonomysim_ros_pkgs/CarControls](https://github.com/nervosys/AutonomySim/blob/master/ros/src/autonomysim_ros_pkgs/msg/CarControls.msg)
Throttle, brake, steering and gear selections for control. Both automatic and manual transmission control possible, see the [`car_joy.py`](https://github.com/nervosys/AutonomySim/blob/master/ros/src/autonomysim_ros_pkgs/scripts/car_joy) script for use.

#### Services

* `/autonomysim_node/VEHICLE_NAME/land` [autonomysim_ros_pkgs/Takeoff](https://docs.ros.org/api/std_srvs/html/srv/Empty.html)

* `/autonomysim_node/takeoff` [autonomysim_ros_pkgs/Takeoff](https://docs.ros.org/api/std_srvs/html/srv/Empty.html)

* `/autonomysim_node/reset` [autonomysim_ros_pkgs/Reset](https://docs.ros.org/api/std_srvs/html/srv/Empty.html) resets *all* drones

#### Parameters

* `/autonomysim_node/world_frame_id` [string]
  Set in: `$(autonomysim_ros_pkgs)/launch/autonomysim_node.launch`
  Default: world_ned
  Set to "world_enu" to switch to ENU frames automatically

* `/autonomysim_node/odom_frame_id` [string]
  Set in: `$(autonomysim_ros_pkgs)/launch/autonomysim_node.launch`
  Default: odom_local_ned
  If you set world_frame_id to "world_enu", the default odom name will instead default to "odom_local_enu"

* `/autonomysim_node/coordinate_system_enu` [boolean]
  Set in: `$(autonomysim_ros_pkgs)/launch/autonomysim_node.launch`
  Default: false
  If you set world_frame_id to "world_enu", this setting will instead default to true

* `/autonomysim_node/update_autonomysim_control_every_n_sec` [double]
  Set in: `$(autonomysim_ros_pkgs)/launch/autonomysim_node.launch`
  Default: 0.01 seconds.
  Timer callback frequency for updating drone odom and state from AutonomySim, and sending in control commands.
  The current RPClib interface to unreal engine maxes out at 50 Hz.
  Timer callbacks in ROS run at maximum rate possible, so it's best to not touch this parameter.

* `/autonomysim_node/update_autonomysim_img_response_every_n_sec` [double]
  Set in: `$(autonomysim_ros_pkgs)/launch/autonomysim_node.launch`
  Default: 0.01 seconds.
  Timer callback frequency for receiving images from all cameras in AutonomySim.
  The speed will depend on number of images requested and their resolution.
  Timer callbacks in ROS run at maximum rate possible, so it's best to not touch this parameter.

* `/autonomysim_node/publish_clock` [double]
  Set in: `$(autonomysim_ros_pkgs)/launch/autonomysim_node.launch`
  Default: false
  Will publish the ros /clock topic if set to true.

### Position Controller Node

#### Parameters

* PD controller parameters:
  * `/pd_position_node/kd_x` [double],
    `/pd_position_node/kp_y` [double],
    `/pd_position_node/kp_z` [double],
    `/pd_position_node/kp_yaw` [double]
    Proportional gains

  * `/pd_position_node/kd_x` [double],
    `/pd_position_node/kd_y` [double],
    `/pd_position_node/kd_z` [double],
    `/pd_position_node/kd_yaw` [double]
    Derivative gains

  * `/pd_position_node/reached_thresh_xyz` [double]
    Threshold euler distance (meters) from current position to setpoint position

  * `/pd_position_node/reached_yaw_degrees` [double]
    Threshold yaw distance (degrees) from current position to setpoint position

* `/pd_position_node/update_control_every_n_sec` [double]
  Default: 0.01 seconds

#### Services

* `/autonomysim_node/VEHICLE_NAME/gps_goal` [Request: [srv/SetGPSPosition](https://github.com/nervosys/AutonomySim/blob/master/ros/src/autonomysim_ros_pkgs/srv/SetGPSPosition.srv)]
  Target gps position + yaw.
  In **absolute** altitude.

* `/autonomysim_node/VEHICLE_NAME/local_position_goal` [Request: [srv/SetLocalPosition](https://github.com/nervosys/AutonomySim/blob/master/ros/src/autonomysim_ros_pkgs/srv/SetLocalPosition.srv)]
  Target local position + yaw in global NED frame.

#### Subscribers

* `/autonomysim_node/origin_geo_point` [autonomysim_ros_pkgs/GPSYaw](https://github.com/nervosys/AutonomySim/tree/master/ros/src/autonomysim_ros_pkgs/msg/GPSYaw.msg)
  Listens to home geo coordinates published by `autonomysim_node`.

* `/autonomysim_node/VEHICLE_NAME/odom_local_ned` [nav_msgs/Odometry](https://docs.ros.org/api/nav_msgs/html/msg/Odometry.html)
  Listens to odometry published by `autonomysim_node`

#### Publishers

* `/vel_cmd_world_frame` [autonomysim_ros_pkgs/VelCmd](https://github.com/nervosys/AutonomySim/tree/master/ros/src/autonomysim_ros_pkgs/msg/VelCmd.msg)
  Sends velocity command to `autonomysim_node`

#### Globals

* Dynamic constraints. These can be changed in `dynamic_constraints.launch`:
  * `/max_vel_horz_abs` [double]
  Maximum horizontal velocity of the drone (meters/second)

  * `/max_vel_vert_abs` [double]
  Maximum vertical velocity of the drone (meters/second)

  * `/max_yaw_rate_degree` [double]
  Maximum yaw rate (degrees/second)

## Miscellaneous

### Configuring the Build Environment on WSL

These setup instructions describe how to setup the `Windows Subsystem for Linux` version 2 (WSL2). It involves enabling WSL2, installing a Linux OS image, and installing the build environment as if it were a native Linux system. Upon completion, you will be able to build and run the ROS wrapper as with any Linux machine.

!!! note
  We recommend using the native file system in `/home/...` rather than Windows-mounted folders under `/mnt/...`, as the former is significantly faster.

#### Setup on WSL

1. Follow the instructions [here](https://docs.microsoft.com/en-us/windows/wsl/install-win10). Ensure that the ROS version you want to use is supported by the Linux/Ubuntu version you want to install.
2. You now have a working Ubuntu subsystem under Windows, you can now go to [Ubuntu 16/18/20/22 instructions](#setup) and then [Running AutonomySim with ROS on WSL](#running-autonomysim-with-ros-on-wsl).

!!! note
    You can run XWindows applications (including SITL) by installing [VcXsrv](https://sourceforge.net/projects/vcxsrv/)  on Windows. To use it find and run `XLaunch` from the Windows start menu. Select `Multiple Windows` in first popup, `Start no client` in second popup, **only** `Clipboard` in third popup. Do **not** select `Native Opengl` (and if you are not able to connect select `Disable access control`). You will need to set the DISPLAY variable to point to your display: in WSL it is `127.0.0.1:0`, in WSL2 it will be the ip address of the PC's network port and can be set by using the code below. Also in WSL2 you may have to disable the firewall for public networks, or create an exception in order for VcXsrv to communicate with WSL2:

    `export DISPLAY=$(cat /etc/resolv.conf | grep nameserver | awk '{print $2}'):0`

!!! tip
    * If you add this line to your ~/.bashrc file you won't need to run this command again
    * For code editing you can install VSCode inside WSL.
    * Windows 10 includes "Windows Defender" virus scanner. It will slow down WSL quite a bit. Disabling it greatly improves disk performance but increases your risk to viruses so disable at your own risk. Here is one of many resources/videos that show you how to disable it: [How to Disable or Enable Windows Defender on Windows 10](https://youtu.be/FmjblGay3AM)

#### File System Access between WSL and Windows 10/11

Inside WSL, Windows drives are referenced in the `/mnt` directory. For example, in order to list documents within your (<username>) documents folder:

`ls /mnt/c/'Documents and Settings'/<username>/Documents`
or
`ls /mnt/c/Users/<username>/Documents`

Inside Windows, the WSL distribution files are located at:

`\\wsl$\<distribution name>` e.g., `\\wsl$\Ubuntu-22.04`

#### Running AutonomySim with ROS on WSL

WSL1:
`export WSL_HOST_IP=127.0.0.1`
WSL2:
`export WSL_HOST_IP=$(cat /etc/resolv.conf | grep nameserver | awk '{print $2}')`

Now, as in the running on Linux [section](#running), execute the following:

```shell
source devel/setup.bash
roslaunch autonomysim_ros_pkgs autonomysim_node.launch output:=screen host:=$WSL_HOST_IP
roslaunch autonomysim_ros_pkgs rviz.launch
```

### Using ROS with Docker

A `Dockerfile` is present in the [`docker`](https://github.com/nervosys/AutonomySim/tree/master/docker/Dockerfile_ros2) directory. To build the `autonomysim-ros` image, run the following:

```shell
cd tools
docker build -t autonomysim-ros -f Dockerfile-ROS .
```

To run, replace the path of the `AutonomySim` folder below:

```shell
docker run --rm -it --net=host -v <your-AutonomySim-folder-path>:/home/testuser/AutonomySim AutonomySim-ros:latest bash
```

The above command mounts the AutonomySim directory to the home directory inside the container. Any changes you make in the source files from your host will be visible inside the container, which is useful for development and testing. Now follow the steps from [Build](#build) to compile and run the ROS wrapper.
