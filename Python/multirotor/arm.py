import setup_path
import AutonomySim

client = AutonomySim.MultirotorClient()
client.confirmConnection()
client.armDisarm(True)
