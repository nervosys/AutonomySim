from autonomysim.clients import VehicleClient
from autonomysim.types import WeatherParameter
from autonomysim.utils import wait_key, SetupPath


def main():
    SetupPath()

    client = VehicleClient()
    client.confirmConnection()

    client.simEnableWeather(True)

    wait_key("Press any key to enable rain at 25%")
    client.simSetWeatherParameter(WeatherParameter.Rain, 0.25)
    wait_key("Press any key to enable rain at 75%")
    client.simSetWeatherParameter(WeatherParameter.Rain, 0.75)
    wait_key("Press any key to enable snow at 50%")
    client.simSetWeatherParameter(WeatherParameter.Snow, 0.50)
    wait_key("Press any key to enable maple leaves at 50%")
    client.simSetWeatherParameter(WeatherParameter.MapleLeaf, 0.50)
    wait_key("Press any key to set all effects to 0%")
    client.simSetWeatherParameter(WeatherParameter.Rain, 0.0)
    client.simSetWeatherParameter(WeatherParameter.Snow, 0.0)
    client.simSetWeatherParameter(WeatherParameter.MapleLeaf, 0.0)
    wait_key("Press any key to enable dust at 50%")
    client.simSetWeatherParameter(WeatherParameter.Dust, 0.50)
    wait_key("Press any key to enable fog at 50%")
    client.simSetWeatherParameter(WeatherParameter.Fog, 0.50)
    wait_key("Press any key to disable all weather effects")

    client.simEnableWeather(False)
    return None


if __name__ == "__main__":
    main()
