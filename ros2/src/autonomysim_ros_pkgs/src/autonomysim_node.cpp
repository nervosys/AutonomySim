// autonomysim_node.cpp

#include "autonomysim_ros_wrapper.h"
#include <rclcpp/rclcpp.hpp>

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    rclcpp::NodeOptions node_options;
    node_options.automatically_declare_parameters_from_overrides(true);
    std::shared_ptr<rclcpp::Node> nh = rclcpp::Node::make_shared("autonomysim_node", node_options);
    std::shared_ptr<rclcpp::Node> nh_img = nh->create_sub_node("img");
    std::shared_ptr<rclcpp::Node> nh_lidar = nh->create_sub_node("lidar");
    std::string host_ip;
    nh->get_parameter("host_ip", host_ip);
    AutonomySimROSWrapper autonomysim_ros_wrapper(nh, nh_img, nh_lidar, host_ip);
    rclcpp::executors::MultiThreadedExecutor executor;
    executor.add_node(nh);
    executor.spin();

    return 0;
}