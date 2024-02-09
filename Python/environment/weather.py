import setup_path
import autonomysim

client = autonomysim.VehicleClient()
client.confirmConnection()

client.simEnableWeather(True)

autonomysim.wait_key("Press any key to enable rain at 25%")
client.simSetWeatherParameter(autonomysim.WeatherParameter.Rain, 0.25)
autonomysim.wait_key("Press any key to enable rain at 75%")
client.simSetWeatherParameter(autonomysim.WeatherParameter.Rain, 0.75)
autonomysim.wait_key("Press any key to enable snow at 50%")
client.simSetWeatherParameter(autonomysim.WeatherParameter.Snow, 0.50)
autonomysim.wait_key("Press any key to enable maple leaves at 50%")
client.simSetWeatherParameter(autonomysim.WeatherParameter.MapleLeaf, 0.50)
autonomysim.wait_key("Press any key to set all effects to 0%")
client.simSetWeatherParameter(autonomysim.WeatherParameter.Rain, 0.0)
client.simSetWeatherParameter(autonomysim.WeatherParameter.Snow, 0.0)
client.simSetWeatherParameter(autonomysim.WeatherParameter.MapleLeaf, 0.0)
autonomysim.wait_key("Press any key to enable dust at 50%")
client.simSetWeatherParameter(autonomysim.WeatherParameter.Dust, 0.50)
autonomysim.wait_key("Press any key to enable fog at 50%")
client.simSetWeatherParameter(autonomysim.WeatherParameter.Fog, 0.50)
autonomysim.wait_key("Press any key to disable all weather effects")

client.simEnableWeather(False)
