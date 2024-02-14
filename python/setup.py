import setuptools
from autonomysim import __version__

with open("README.md", "r") as fh:
    long_description = fh.read()

setuptools.setup(
    name="autonomysim",
    version=__version__,
    author="Adam Erickson",
    author_email="opensource@nervosys.ai",
    description="The simulation engine for autonomous systems",
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
