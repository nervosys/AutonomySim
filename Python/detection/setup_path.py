# Import this module to automatically setup path to local AutonomySim module
# This module first tries to see if AutonomySim module is installed via pip
# If it does then we don't do anything else
# Else we look up grand-parent folder to see if it has AutonomySim folder
#    and if it does then we add that in sys.path

import os
import sys
import logging

#this class simply tries to see if AutonomySim 
class SetupPath:
    @staticmethod
    def getDirLevels(path):
        path_norm = os.path.normpath(path)
        return len(path_norm.split(os.sep))

    @staticmethod
    def getCurrentPath():
        cur_filepath = __file__
        return os.path.dirname(cur_filepath)

    @staticmethod
    def getGrandParentDir():
        cur_path = SetupPath.getCurrentPath()
        if SetupPath.getDirLevels(cur_path) >= 2:
            return os.path.dirname(os.path.dirname(cur_path))
        return ''

    @staticmethod
    def getParentDir():
        cur_path = SetupPath.getCurrentPath()
        if SetupPath.getDirLevels(cur_path) >= 1:
            return os.path.dirname(cur_path)
        return ''

    @staticmethod
    def addAutonomySimModulePath():
        # if AutonomySim module is installed then don't do anything else
        #import pkgutil
        #AutonomySim_loader = pkgutil.find_loader('AutonomySim')
        #if AutonomySim_loader is not None:
        #    return

        parent = SetupPath.getParentDir()
        if parent !=  '':
            AutonomySim_path = os.path.join(parent, 'AutonomySim')
            client_path = os.path.join(AutonomySim_path, 'client.py')
            if os.path.exists(client_path):
                sys.path.insert(0, parent)
        else:
            logging.warning("AutonomySim module not found in parent folder. Using installed package (pip install AutonomySim).")

SetupPath.addAutonomySimModulePath()
