# PFM Image File Format

The Portable FloatMap (PFM) file format stores images with floating-point pixels and hence is not restricted to the 8-bit unsigned integer value range of 0-255. This is useful for HDR images or images that describes something other than colors, such as depth.

A good viewer for this file format is [PfmPad](https://sourceforge.net/projects/pfmpad/). We do not recommend the `Maverick` photo viewer because it doesn't display depth images properly.

`AutonomySim` provides code to write `pfm` files in [C++](https://github.com/nervosys/AutonomySim/blob/main/AutonomyLib/include/common/common_utils/Utils.hpp#L637) and, to read and write `pfm` files in [Python](https://github.com/nervosys/AutonomySim/tree/master/PythonClient//AutonomySim/utils.py#L122).
