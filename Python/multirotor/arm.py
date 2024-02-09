import setup_path
import autonomysim

client = autonomysim.MultirotorClient()
client.confirmConnection()
client.armDisarm(True)
