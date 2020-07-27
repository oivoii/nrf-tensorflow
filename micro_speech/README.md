# Tensorflow Micro Speech feat. I2S
Recognice the words "yes" and "no", and flash LEDs + transmit words as BLE keyboard.

## I2S
The I2S driver is based on this [I2S driver example](https://github.com/siguhe/NCS_I2S_nrfx_driver_example/blob/master/README.md). I2S will most likely work if you do not follow the short instructions found in the README of that examle. 

## Manual edits
For the project to build, in CMakeLists.txt, you need switch YOUR_PATH_TO with your path to the nrf-tensorflow folder. 

## Build and flash
To use this example, the same method as explained in this [nRF Connect SDK Tutorial](https://devzone.nordicsemi.com/nordic/nrf-connect-sdk-guides/b/getting-started/posts/ncs-tutorial---temporary) could be used. 
TL;DR build and flash: `west build -b nrf5340pdk_nrf5340_cpuappns -p` + `west flash`
