import os
import sys
import inspect
import logging

description = """
Utilities
"""


class SetupPath:
    """
    Import this module to automatically setup path to local AutonomySim module.

    This module first tries to see if AutonomySim module is installed via `pip`.
    If it does, we don't do anything else. Else, we look up grand-parent folder to
    see if it has AutonomySim folder and if it does then we add that in the `sys.path`.
    """

    def __init__(self) -> None:
        self.addAutonomySimModulePath()

    @staticmethod
    def getDirLevels(path):
        path_norm = os.path.normpath(path)
        return len(path_norm.split(os.sep))

    @staticmethod
    def getCurrentPath():
        cur_filepath = os.path.abspath(inspect.getfile(inspect.currentframe()))
        return os.path.dirname(cur_filepath)

    @staticmethod
    def getGrandParentDir():
        cur_path = SetupPath.getCurrentPath()
        if SetupPath.getDirLevels(cur_path) >= 2:
            return os.path.dirname(os.path.dirname(cur_path))
        return ""

    @staticmethod
    def getParentDir():
        cur_path = SetupPath.getCurrentPath()
        if SetupPath.getDirLevels(cur_path) >= 1:
            return os.path.dirname(cur_path)
        return ""

    @staticmethod
    def addAutonomySimModulePath():
        # if AutonomySim module is installed then don't do anything else
        # import pkgutil
        # autonomysim_loader = pkgutil.find_loader('AutonomySim')
        # if autonomysim_loader is not None:
        #    return

        parent = SetupPath.getParentDir()
        if parent != "":
            autonomysim_path = os.path.join(parent, "AutonomySim")
            client_path = os.path.join(autonomysim_path, "client.py")
            if os.path.exists(client_path):
                sys.path.insert(0, parent)
        else:
            logging.warning(
                "AutonomySim module not found in parent folder. Using installed package (pip install AutonomySim)."
            )


def get_public_fields(obj):
    return [
        attr
        for attr in dir(obj)
        if not (
            attr.startswith("_")
            or inspect.isbuiltin(attr)
            or inspect.isfunction(attr)
            or inspect.ismethod(attr)
        )
    ]


def wait_key(message=""):
    """Wait for a key press on the console and return it."""
    if message != "":
        print(message)

    result = None
    if os.name == "nt":
        import msvcrt

        result = msvcrt.getch()
    else:
        import termios

        fd = sys.stdin.fileno()
        oldterm = termios.tcgetattr(fd)
        newattr = termios.tcgetattr(fd)
        newattr[3] = newattr[3] & ~termios.ICANON & ~termios.ECHO
        termios.tcsetattr(fd, termios.TCSANOW, newattr)

        try:
            result = sys.stdin.read(1)
        except IOError:
            pass
        finally:
            termios.tcsetattr(fd, termios.TCSAFLUSH, oldterm)

    return result


def generate_color_palette(numPixelsWide, outputFile):
    import random
    import numpy as np
    import cv2

    random.seed(42)

    palette = np.zeros((1, 256 * numPixelsWide, 3))
    possibilities = [list(range(256)), list(range(256)), list(range(256))]

    colors = [[0] * 3 for i in range(256)]
    choice = 0
    j = 0
    for i in range(3):
        palette[0, j * numPixelsWide : (j + 1) * numPixelsWide, i] = choice
        colors[j][i] = choice

    for i in range(3):
        for j in range(1, 255):
            choice = random.sample(possibilities[i], 1)[0]
            possibilities[i].remove(choice)
            palette[0, j * numPixelsWide : (j + 1) * numPixelsWide, i] = choice
            colors[j][i] = choice

    choice = 255
    j = 255
    for i in range(3):
        palette[0, j * numPixelsWide : (j + 1) * numPixelsWide, i] = choice
        colors[j][i] = choice

    cv2.imwrite(outputFile, palette, [cv2.IMWRITE_PNG_COMPRESSION, 0])

    rgb_file = open("rgbs.txt", "w")
    for j in range(256):
        rgb_file.write("%d\t%s\n" % (j, str(list(reversed(colors[j])))))
    rgb_file.close()
