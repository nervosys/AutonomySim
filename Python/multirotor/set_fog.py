import setup_path
import AutonomySim

import time

client = AutonomySim.MultirotorClient()
client.confirmConnection()
client.enableApiControl(True)

client.armDisarm(True)

client.simEnableWeather(True)

print("Setting fog to 25%")
client.simSetWeatherParameter(AutonomySim.WeatherParameter.Fog, 0.25)

# Takeoff or hover
landed = client.getMultirotorState().landed_state
if landed == AutonomySim.LandedState.Landed:
    print("taking off...")
    client.takeoffAsync().join()
else:
    print("already flying...")
    client.hoverAsync().join()

time.sleep(5)

print("Setting fog to 50%")
client.simSetWeatherParameter(AutonomySim.WeatherParameter.Fog, 0.5)

time.sleep(5)

print("Resetting fog to 0%")
client.simSetWeatherParameter(AutonomySim.WeatherParameter.Fog, 0)
