# ROS Packages Tutorial

This is a set of example `settings.json`, `roslaunch`, and `rviz` files to provide a starting point for using `AutonomySim` with `ROS`. See [autonomysim_ros_pkgs](https://github.com/nervosys/AutonomySim/blob/master/ros2/src/autonomysim_ros_pkgs/README.md) for the ROS API.

!!! note "`tf`: ROS transforms"
  We use the abbreviation `tf` herein to refer to ROS transforms, as provided by the [ROS tf2 library](http://wiki.ros.org/tf2).

## Setup

Make sure that the [autonomysim_ros_pkgs](ros_pkgs.md) setup has been completed and the prerequisites installed.

```shell
cd PATH_TO/AutonomySim/ros
catkin build autonomysim_tutorial_pkgs
```

If your default `gcc` is less than or equal to version 8 (see `gcc --version` output), compilation will fail. In that case, use `gcc-8` explicitly as follows:

```shell
catkin build autonomysim_tutorial_pkgs -DCMAKE_C_COMPILER=gcc-8 -DCMAKE_CXX_COMPILER=g++-8
```

!!! note
    When running examples and opening a new terminal, sourcing the `setup.bash` file is necessary. If you're using the ROS wrapper frequently, it might be helpful to add the `source PATH_TO/AutonomySim/ros/devel/setup.bash` to your `~/.profile` or `~/.bashrc` to avoid needing to run this command every time a new terminal is opened.

## Examples

### Single drone with monocular and depth cameras, and LiDAR

* `settings.json` [front_stereo_and_center_mono.json](https://github.com/nervosys/AutonomySim/blob/master/ros/src/autonomysim_tutorial_pkgs/settings/front_stereo_and_center_mono.json)

  ```shell
  source PATH_TO/AutonomySim/ros/devel/setup.bash
  roscd autonomysim_tutorial_pkgs
  cp settings/front_stereo_and_center_mono.json ~/Documents/AutonomySim/settings.json
 
  # Start your unreal package or binary here
  roslaunch autonomysim_ros_pkgs autonomysim_node.launch;
 
  # in a new pane / terminal
  source PATH_TO/AutonomySim/ros/devel/setup.bash
  roslaunch autonomysim_tutorial_pkgs front_stereo_and_center_mono.launch
  ```

  The above would start `rviz` with `tf`s, registered RGB-D cloud using [depth_image_proc](https://wiki.ros.org/depth_image_proc) using the [`depth_to_pointcloud` launch file](https://github.com/nervosys/AutonomySim/blob/master/ros/src/autonomysim_tutorial_pkgs/launch/front_stereo_and_center_mono/depth_to_pointcloud.launch), and the LiDAR point cloud. 

### Multi-drone with cameras, LiDARs, and IMUs

Where `N = 2` in this case.

* `settings.json`: [two_drones_camera_lidar_imu.json](https://github.com/nervosys/AutonomySim/blob/master/ros/src/autonomysim_tutorial_pkgs/settings/two_drones_camera_lidar_imu.json) 

  ```shell
  source PATH_TO/AutonomySim/ros/devel/setup.bash
  roscd autonomysim_tutorial_pkgs
  cp settings/two_drones_camera_lidar_imu.json ~/Documents/AutonomySim/settings.json
 
  # Start your unreal package or binary here
  roslaunch autonomysim_ros_pkgs autonomysim_node.launch;
  roslaunch autonomysim_ros_pkgs rviz.launch
  ```

You can view the `tf`s in `rviz`. And do a `rostopic list` and `rosservice list` to inspect the services avaiable.    

### Twenty-five drones in a square pattern

* `settings.json`: [twenty_five_drones.json](https://github.com/nervosys/AutonomySim/blob/master/ros/src/autonomysim_tutorial_pkgs/settings/twenty_five_drones.json) 

  ```shell
  source PATH_TO/AutonomySim/ros/devel/setup.bash
  roscd autonomysim_tutorial_pkgs
  cp settings/twenty_five_drones.json ~/Documents/AutonomySim/settings.json
 
  # Start your unreal package or binary here
  roslaunch autonomysim_ros_pkgs autonomysim_node.launch;
  roslaunch autonomysim_ros_pkgs rviz.launch
  ```

You can view the `tf`s in `rviz`. And do a `rostopic list` and `rosservice list` to inspect the services avaiable.
