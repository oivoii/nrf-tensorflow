Disclaimer: This project is under developement, and can not be promised to be stable or even functional.


# nrf-tensorflow
Tensorflow on nRF with NCS
Use NCS to include [Tensroflow Lite Micro](https://github.com/tensorflow/tensorflow/tree/master/tensorflow/lite/micro) as a library.

## Operating system
As far as I know, Tensorflow Lite Micro is made for use with Linux. 

### Windows
If you are using Windows, you are mostly on your own. However, [this](https://www.wikihow.com/Install-Ubuntu-on-VirtualBox) might be a good place to begin.

## Install
After you clone this repository you need to load the tensorflow submodule>
```
git submodule init
git submodule update
```

You also need to install the [NCS](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/gs_installing.html) repository.


## Tensorflow
Oh, and since I can not get CmakeLists.txt to make make downloads external libraries when compilin, you need to manually run:
```
cd [this_git_home]/tensorflow
make -f tensorflow/lite/micro/tools/make/Makefile third_party_downloads
```

## Versions
NCS v1.3.0
Tested on nRF5340PDK 0.8.0 and nRF9160DK 0.8.5
