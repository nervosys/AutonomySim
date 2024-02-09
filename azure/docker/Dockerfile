FROM nvidia/cudagl:11.4.0-devel

RUN apt-get update
RUN apt-get install \
	sudo \
	libglu1-mesa-dev \
	xdg-user-dirs \
	pulseaudio \
	sudo \
	x11-xserver-utils \
	unzip \
	wget \
	software-properties-common \
	-y --no-install-recommends

RUN apt-add-repository ppa:deadsnakes/ppa
RUN apt-get update
RUN apt-get install -y \
	python3.11 \
	python3-pip 

RUN python3.11 -m pip install --upgrade pip
RUN python3.11 -m pip install setuptools wheel

RUN adduser --force-badname --disabled-password --gecos '' --shell /bin/bash autonomysim_user && \ 
	echo '%sudo ALL=(ALL) NOPASSWD:ALL' >> /etc/sudoers && \ 
	adduser autonomysim_user sudo && \ 
	adduser autonomysim_user audio && \ 
	adduser autonomysim_user video

USER autonomysim_user
WORKDIR /home/autonomysim_user

# Change the following values to use a different AutonomySim binary
# Also change the AUTONOMYSIM_EXECUTABLE variable in docker-entrypoint.sh
ENV AUTONOMYSIM_BINARY_ZIP_URL=https://github.com/nervosys/AutonomySim/releases/download/v1.3.1-linux/Blocks.zip 
ENV AUTONOMYSIM_BINARY_ZIP_FILENAME=Blocks.zip

ENV SDL_VIDEODRIVER_VALUE=offscreen
ENV SDL_HINT_CUDA_DEVICE=0

# Download and unzip the AutonomySim binary
RUN wget -c $AUTONOMYSIM_BINARY_ZIP_URL
RUN unzip $AUTONOMYSIM_BINARY_ZIP_FILENAME
RUN rm $AUTONOMYSIM_BINARY_ZIP_FILENAME

# Add the Python app to the image
ADD ./app /home/autonomysim_user/app

WORKDIR /home/autonomysim_user
RUN mkdir -p /home/autonomysim_user/Documents/AutonomySim
ADD ./app/settings.json /home/autonomysim_user/Documents/AutonomySim
ADD ./docker/docker-entrypoint.sh /home/autonomysim_user/docker-entrypoint.sh

RUN sudo chown -R autonomysim_user /home/autonomysim_user

WORKDIR /home/autonomysim_user/app

# Install Python requirements
RUN python3.11 -m pip install -r requirements.txt

ENTRYPOINT /home/autonomysim_user/docker-entrypoint.sh
