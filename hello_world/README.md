# Tensorflow Hello World feat. pwm
Will use AI to make led pwm as a sine wave.

## Manual edits
For the project to build, in CMakeLists.txt, you need switch YOUR_PATH_TO with your path to the nrf-tensorflow folder. 

## Build and flash
To use this example, the same method as explained in this [nRF Connect SDK Tutorial](https://devzone.nordicsemi.com/nordic/nrf-connect-sdk-guides/b/getting-started/posts/ncs-tutorial---temporary) could be used. 
TL;DR build and flash: `west build -b nrf5340pdk_nrf5340_cpuapp -p` + `west flash`

## Versions
NCS v1.3.0.
Tested on nRF5340PDK 0.8.0, nRF9160DK 0.8.5 and nRF52840DK 2.0.0.

## nRF52840DK
nRF52840 has a Cortex-m4 instead of Cortex-m33. Because of this, you need to change TARGET_ARCH in CMakeLists.txt. 
When building, we get an error saying that "ncs/nrfxlib/crypto/nrf_cc310_platform/lib/cortex-m4/softfp-float does not exist". This can be fixed by copying the soft-float folder to softfp-float. This have to be done manually. 
