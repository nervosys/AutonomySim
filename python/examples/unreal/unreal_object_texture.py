from autonomysim.vehicles.air import MultirotorClient


def main():
    c = MultirotorClient()
    c.confirmConnection()
    c.simSetObjectMaterialFromTexture("OrangeBall", "sample_texture.jpg")


if __name__ == "__main__":
    main()
