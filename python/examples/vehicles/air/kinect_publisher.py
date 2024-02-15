import rospy
from sensor_msgs.msg import Image, CameraInfo
from tf2_msgs.msg import TFMessage

from autonomysim.types import ImageRequest, ImageType
from autonomysim.clients import MultirotorClient
from autonomysim.sensors.kinect import KinectPublisher


# when enabled, RGB image is enhanced using CLAHE
CLAHE_ENABLED = False


def main():
    client = MultirotorClient()
    client.confirmConnection()
    client.enableApiControl(True)
    client.armDisarm(True)
    rospy.init_node("autonomysim_publisher", anonymous=True)
    publisher_d = rospy.Publisher(
        "/camera/depth_registered/image_raw", Image, queue_size=1
    )
    publisher_rgb = rospy.Publisher("/camera/rgb/image_rect_color", Image, queue_size=1)
    publisher_info = rospy.Publisher(
        "/camera/rgb/camera_info", CameraInfo, queue_size=1
    )
    publisher_tf = rospy.Publisher("/tf", TFMessage, queue_size=1)
    rate = rospy.Rate(30)  # 30 Hz
    pub = KinectPublisher()

    while not rospy.is_shutdown():
        responses = client.simGetImages(
            [
                ImageRequest(0, ImageType.DepthPlanar, True, False),
                ImageRequest(0, ImageType.Scene, False, False),
            ]
        )
        img_depth = pub.getDepthImage(responses[0])
        img_rgb = pub.getRGBImage(responses[1])

        if CLAHE_ENABLED:
            img_rgb = pub.enhanceRGB(img_rgb)

        pub.GetCurrentTime()
        msg_rgb = pub.CreateRGBMessage(img_rgb)
        msg_d = pub.CreateDMessage(img_depth)
        msg_info = pub.CreateInfoMessage()
        msg_tf = pub.CreateTFMessage()

        publisher_rgb.publish(msg_rgb)
        publisher_d.publish(msg_d)
        publisher_info.publish(msg_info)
        publisher_tf.publish(msg_tf)

        del pub.msg_info.D[:]
        del pub.msg_tf.transforms[:]

        rate.sleep()


if __name__ == "__main__":
    main()
