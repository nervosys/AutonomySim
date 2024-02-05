import AutonomySim

c = AutonomySim.MultirotorClient()
c.confirmConnection()
c.simSetObjectMaterialFromTexture("OrangeBall", "sample_texture.jpg")
