# Installing CMake on Linux

If you don't have CMake version 3.10 or greater, you can run the following to install it:

```bash
mkdir ~/cmake-3.10.2
cd ~/cmake-3.10.2
wget https://cmake.org/files/v3.10/cmake-3.10.2-Linux-x86_64.sh
```

Now, you have to run this command by itself (it is interactive):

```bash
sh cmake-3.10.2-Linux-x86_64.sh --prefix ~/cmake-3.10.2
```

Answer `n` to the question about creating another `cmake-3.10.2-Linux-x86_64` folder and then this:

```bash
sudo update-alternatives --install /usr/bin/cmake cmake ~/cmake-3.10.2/bin/cmake 60
```

Now type `cmake --version` to ensure your CMake version is equal to `3.10.2`.
