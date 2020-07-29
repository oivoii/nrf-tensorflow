# Tensorflow Hello World feat. pwm
Will use AI to make led pwm as a sine wave.

## Manual edits
For the project to build, in CMakeLists.txt, you need switch YOUR_PATH_TO with your path to the nrf-tensorflow folder. 

## Build and flash
To use this example, the same method as explained in this [nRF Connect SDK Tutorial](https://devzone.nordicsemi.com/nordic/nrf-connect-sdk-guides/b/getting-started/posts/ncs-tutorial---temporary) could be used. 
TL;DR build and flash: `west build -b nrf5340pdk_nrf5340_cpuapp -p` + `west flash`
