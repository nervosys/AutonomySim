import setup_path 
import AutonomySim

import time

client = AutonomySim.MultirotorClient()
client.confirmConnection()

pose = client.simGetVehiclePose()

# teleport the drone + 10 meters in x-direction
pose.position.x_val += 10

client.simSetVehiclePose(pose, True, "PX4")

time.sleep(2)

# teleport the drone back
pose.position.x_val -= 10

client.simSetVehiclePose(pose, True, "PX4")
