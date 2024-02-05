import setuptools
from AutonomySim import __version__

with open("README.md", "r") as fh:
    long_description = fh.read()

setuptools.setup(
    name="AutonomySim",
    version=__version__,
    author="Adam Erickson",
    author_email="opensource@nervosys.ai",
    description="Open-source simulator for autonomous systems from Nervosys AI",
    long_description=long_description,
    long_description_content_type="text/markdown",
    url="https://github.com/nervosys/AutonomySim",
    packages=setuptools.find_packages(),
    license="Apache 2.0",
    classifiers=(
        "Programming Language :: Python :: 3",
        "License :: OSI Approved :: Apache 2.0 License",
        "Operating System :: OS Independent",
    ),
    install_requires=["msgpack-rpc-python", "numpy", "opencv-contrib-python"],
)
