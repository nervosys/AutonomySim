import setup_path 
import AutonomySim

client = AutonomySim.MultirotorClient()
client.confirmConnection()
client.enableApiControl(True)
client.armDisarm(True)

landed = client.getMultirotorState().landed_state
if landed == AutonomySim.LandedState.Landed:
    print("already landed...")
else:
    print("landing...")
    client.landAsync().join()

client.armDisarm(False)
client.enableApiControl(False)
