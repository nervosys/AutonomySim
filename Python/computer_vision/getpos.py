# In settings.json first activate computer vision mode: 
# https://github.com/nervosys/AutonomySim/blob/main/docs/image_apis.md#computer-vision-mode

import setup_path 
import AutonomySim

client = AutonomySim.VehicleClient()
client.confirmConnection()

pose = client.simGetVehiclePose()
print("x={}, y={}, z={}".format(pose.position.x_val, pose.position.y_val, pose.position.z_val))

angles = AutonomySim.to_eularian_angles(client.simGetVehiclePose().orientation)
print("pitch={}, roll={}, yaw={}".format(angles[0], angles[1], angles[2]))

pose.position.x_val = pose.position.x_val + 1
client.simSetVehiclePose(pose, True)