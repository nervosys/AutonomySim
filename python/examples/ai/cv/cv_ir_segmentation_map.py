import os
import sys
import time
import random
import numpy as np
import cv2

from autonomysim.clients import MultirotorClient
from python.autonomysim.sensors.thermal_camera import (
    get_new_temp_emiss_from_radiance,
    set_segmentation_ids,
)


if __name__ == "__main__":
    # Connect to autonomysim, UAV mode
    client = MultirotorClient()
    client.confirmConnection()

    segIdDict = {
        "Base_Terrain": "soil",
        "elephant": "elephant",
        "zebra": "zebra",
        "Crocodile": "crocodile",
        "Rhinoceros": "rhinoceros",
        "Hippo": "hippopotamus",
        "Poacher": "human",
        "InstancedFoliageActor": "tree",
        "Water_Plane": "water",
        "truck": "truck",
    }

    # Choose temperature values for winter or summer.
    # """
    # winter
    tempEmissivity = np.array(
        [
            ["elephant", 290, 0.96],
            ["zebra", 298, 0.98],
            ["rhinoceros", 291, 0.96],
            ["hippopotamus", 290, 0.96],
            ["crocodile", 295, 0.96],
            ["human", 292, 0.985],
            ["tree", 273, 0.952],
            ["grass", 273, 0.958],
            ["soil", 278, 0.914],
            ["shrub", 273, 0.986],
            ["truck", 273, 0.8],
            ["water", 273, 0.96],
        ]
    )
    # """
    """
    #summer
    tempEmissivity = np.array([['elephant',298,0.96], 
                               ['zebra',307,0.98],
                               ['rhinoceros',299,0.96],
                               ['hippopotamus',298,0.96],
                               ['crocodile',303,0.96],
                               ['human',301,0.985], 
                               ['tree',293,0.952], 
                               ['grass',293,0.958], 
                               ['soil',288,0.914], 
                               ['shrub',293,0.986],
                               ['truck',293,0.8],
                               ['water',293,0.96]])
    """

    # Read camera response.
    response = None
    camResponseFile = "camera_response.npy"
    try:
        np.load(camResponseFile)
    except Exception:
        print("{} not found. Using default response.".format(camResponseFile))

    # Calculate radiance.
    tempEmissivityNew = get_new_temp_emiss_from_radiance(tempEmissivity, response)

    # Set IDs in autonomysim environment.
    set_segmentation_ids(segIdDict, tempEmissivityNew, client)
