import time

from autonomysim.types import WeatherParameter, LandedState
from autonomysim.clients import MultirotorClient
from autonomysim.utils import SetupPath


def main():
    SetupPath()

    client = MultirotorClient()
    client.confirmConnection()
    client.enableApiControl(True)

    client.armDisarm(True)

    client.simEnableWeather(True)

    print("Setting fog to 25%")
    client.simSetWeatherParameter(WeatherParameter.Fog, 0.25)

    # Takeoff or hover
    landed = client.getMultirotorState().landed_state
    if landed == LandedState.Landed:
        print("taking off...")
        client.takeoffAsync().join()
    else:
        print("already flying...")
        client.hoverAsync().join()

    time.sleep(5)

    print("Setting fog to 50%")
    client.simSetWeatherParameter(WeatherParameter.Fog, 0.5)

    time.sleep(5)

    print("Resetting fog to 0%")
    client.simSetWeatherParameter(WeatherParameter.Fog, 0)


if __name__ == "__main__":
    main()
