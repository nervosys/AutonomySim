# Cloud Development Environments

This document explains how to automate the creation of a `Microsoft Azure` cloud development environment (MSA-CDE) to code and debug a Python application connected to `AutonomySim` using Visual Studio Code.

## Automatically Deploy the Azure virtual machine (VM)

Click the blue button to start the Azure deployment.

!!! note
    The template is pre-filled with the recommended virtual machine size for the use cases of the following two tutorials.

<a href="https://aka.ms/AA8umgt" target="_blank">
    <img src="https://azuredeploy.net/deploybutton.png"/>
</a>  

!!! note
    The VM deployment and configuration process may take over 20 minutes to complete.

### Regarding the deployment of the Azure VM

* When using an Azure Trial account, the default vCPU quota is not enough to allocate the required VM to run AutonomySim. If that's the case, you will see an error when trying to create the VM and will have to submit a request for Quota increase. **Be sure to understand how and how much you are going to be charged for the use of the VM**

* To avoid charges for the Virtual Machine usage while not in use, remember to deallocate its resources from the [Azure Portal](https://portal.azure.com) or use the following command from the Azure CLI:

```shell
az vm deallocate --resource-group MyResourceGroup --name MyVMName
```

## Code and debug from Visual Studio Code and Remote SSH

* Install Visual Studio Code
* Install the *Remote - SSH* extension
* Press `F1` and run the `Remote - SSH: Connect to host...` command
* Add the recently create VM details. For instance, `AzureUser@11.22.33.44`
* Run the `Remote - SSH: Connect to host...` command again, and now select the newly added connection.
* Once connected, click on the `Clone Repository` button in Visual Studio Code, and either clone this repository in the remote VM and open *just the `azure` folder*, or create a brand new repository, clone it and copy the contents of the `azure` folder from this repository in it. It is important to open that directory so Visual Studio Code can use the specific `.vscode` directory for the scenario and not the general AutonomySim `.vscode` directory. It contains the recommended extensions to install, the task to start AutonomySim remotely and the launch configuration for the Python application.
* Install all the recommended extensions
* Press `F1` and select the `Tasks: Run Task` option. Then, select the `Start AutonomySim` task from Visual Studio Code to execute the `start-AutonomySim.ps1` script from Visual Studio Code.
* Open the `multirotor.py` file inside the `app` directory
* Start debugging with Python
* When finished, remember to stop an deallocate the Azure VM to avoid extra charges

## Code and debug on local and connect to remote via forwarded ports

!!! note
    In this scenario, we use two Visual Studio Code instances. The first will be used as a bridge to forward ports via SSH to the Azure VM and execute remote processes and the second will be used for local Python development. To be able to reach the VM from the local Python code, it is required to keep the `Remote - SSH` instance of Visual Studio Code opened while working with the local Python environment on the second instance.

* Open the first Visual Studio Code instance
* Follow the steps in the previous section to connect via `Remote - SSH`
* In the *Remote Explorer*, add the port `41451` as a forwarded port to localhost
* Either run the `Start AutonomySim` task on the Visual Studio Code with the remote session as explained in the previous scenario or manually start the AutonomySim binary in the VM
* Open a second Visual Studio Code instance, without disconnecting or closing the first one
* Either clone this repository locally and open *just the `azure` folder* in Visual Studio Code, or create a brand new repository, clone it and copy the contents of the `azure` folder from this repository in it.
* Run `pip install -r requirements.txt` inside the `app` directory
* Open the `multirotor.py` file inside the `app` directory 
* Start debugging with Python
* When finished, remember to stop an deallocate the Azure VM to avoid extra charges

## Running with Docker

Once both the AutonomySim environment and the Python application are ready, you can package everything as a Docker image. The sample project inside the `azure` directory is already prepared to run a prebuilt AutonomySim binary and Python code using Docker.

This would be a perfect scenario when you want to run the simulation at scale. For instance, you could have several different configurations for the same simulation and execute them in a parallel, unattended way using a Docker image on Azure Container Services

Since AutonomySim requires access to the host GPU, it is required to use a Docker runtime that supports it. For more information about running AutonomySim in Docker, click [here](docker_ubuntu.md).

When using Azure Container Services to run this image, the only extra-requirement is to add GPU support to the Container Group where it will be deployed. 

It can use either public docker images from DockerHub or images deployed to a private Azure Container Registry

### Building the docker image

```shell
docker build -t <your-registry-url>/<your-image-name> -f ./docker/Dockerfile .`
```

## Using a different AutonomySim binary

To use a different `AutonomySim` binary, first check the official documentation on [How to Build AutonomySim on Windows](build_windows.md) and [How to Build AutonomySim on Linux](build_linux.md) if you also want to run it with Docker

Once you have a zip file with the new AutonomySim environment (or prefer to use one from the [Official Releases](https://github.com/nervosys/AutonomySim/releases)), you need to modify some of the scripts in the `azure` directory of the repository to point to the new environment:

* In [`azure/azure-env-creation/configure-vm.ps1`](https://github.com/nervosys/AutonomySim/blob/master/azure/azure-env-creation/configure-vm.ps1), modify all the parameters starting with `$AutonomySimBinary` with the new values
* In [`azure/start-AutonomySim.ps1`](https://github.com/nervosys/AutonomySim/blob/master/azure/start-AutonomySim.ps1), modify `$AutonomySimExecutable` and `$AutonomySimProcessName` with the new values

If you are using the docker image, you also need a Linux binary zip file and modify the following Docker-related files:

* In [`azure/docker/Dockerfile`](https://github.com/nervosys/AutonomySim/blob/master/azure/docker/Dockerfile), modify the `AUTONOMYSIM_BINARY_ZIP_URL` and `AUTONOMYSIM_BINARY_ZIP_FILENAME` ENV declarations with the new values
* In [`azure/docker/docker-entrypoint.sh`](https://github.com/nervosys/AutonomySim/blob/master/azure/docker/docker-entrypoint.sh), modify `AUTONOMYSIM_EXECUTABLE` with the new value 

## Maintaining the development environment

Several components of this development environment (ARM templates, initialization scripts and VSCode tasks) directly depend on the current directory structures file names and repository locations. When planning to modify/fork any of those, make sure to check every script and template to make any required adjustment.
