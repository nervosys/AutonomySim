import setup_path 
import AutonomySim

client = AutonomySim.VehicleClient()
client.confirmConnection()

client.simEnableWeather(True)

AutonomySim.wait_key('Press any key to enable rain at 25%')
client.simSetWeatherParameter(AutonomySim.WeatherParameter.Rain, 0.25);

AutonomySim.wait_key('Press any key to enable rain at 75%')
client.simSetWeatherParameter(AutonomySim.WeatherParameter.Rain, 0.75);

AutonomySim.wait_key('Press any key to enable snow at 50%')
client.simSetWeatherParameter(AutonomySim.WeatherParameter.Snow, 0.50);

AutonomySim.wait_key('Press any key to enable maple leaves at 50%')
client.simSetWeatherParameter(AutonomySim.WeatherParameter.MapleLeaf, 0.50);

AutonomySim.wait_key('Press any key to set all effects to 0%')
client.simSetWeatherParameter(AutonomySim.WeatherParameter.Rain, 0.0);
client.simSetWeatherParameter(AutonomySim.WeatherParameter.Snow, 0.0);
client.simSetWeatherParameter(AutonomySim.WeatherParameter.MapleLeaf, 0.0);

AutonomySim.wait_key('Press any key to enable dust at 50%')
client.simSetWeatherParameter(AutonomySim.WeatherParameter.Dust, 0.50);

AutonomySim.wait_key('Press any key to enable fog at 50%')
client.simSetWeatherParameter(AutonomySim.WeatherParameter.Fog, 0.50);

AutonomySim.wait_key('Press any key to disable all weather effects')
client.simEnableWeather(False)
