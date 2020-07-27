# Tensorflow Hello World feat. pwm
Will use AI to make led pwm as a sine wave.
I have tested it on both nrf5340dk and nrf9160dk

## NCS
As of v1.3.0, the nRF5340pdk does not have built in PWM support, so if you want to run hello world on that,  you need to add pwm to the Device Tree of nrf5340.

## Manual edits
For the project to build, in CMakeLists.txt, you need switch YOUR_PATH_TO with your path to the nrf-tensorflow folder. 

## Build and flash
To use this example, the same method as explained in this [nRF Connect SDK Tutorial](https://devzone.nordicsemi.com/nordic/nrf-connect-sdk-guides/b/getting-started/posts/ncs-tutorial---temporary) could be used. 
TL;DR build and flash: `west build -b nrf5340pdk_nrf5340_cpuappns -p` + `west flash`
