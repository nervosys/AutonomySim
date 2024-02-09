import autonomysim

c = autonomysim.MultirotorClient()
c.confirmConnection()
c.simSetObjectMaterialFromTexture("OrangeBall", "sample_texture.jpg")
