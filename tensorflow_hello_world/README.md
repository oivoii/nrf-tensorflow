# Tensorflow Hello World feat. pwm
Will use AI to make led pwm as a sine wave.
I have tested it on both nrf5340dk and nrf9160dk

## NCS
As of v1.3.0, the nRF5340pdk does not have built in PWM support, so if you want to run hello world on that,  you need to add pwm to the Device Tree of nrf5340.
nRF52 has a Cortex-m4 instead of Cortex-m33. for some reason. Therefore, we changed TARGET\_ARCH in CMakeLists.txt.
However, now we get an error saying that nrfxlib/crypto/nrf\_cc310\_platform/lib/cortex-m4/softfp-float does not exist. 
For now, I was able to fix it by copying the soft-float folder to softfp-float. So this have to be done manually. We might.

## nRF52 branch
This project is mainly intended for the nRF53, so this branch probably will not see very much developement.

## Zephyr
This example is based on the zephyr/samples/application\_development/external\_lib
 
## Build and flash
To use this example, the same method as explained in this (nRF Connect SDK Tutorial )[https://devzone.nordicsemi.com/nordic/nrf-connect-sdk-guides/b/getting-started/posts/ncs-tutorial---temporary] could be used. 
TL;DR build and flash: `west build -b [board name here] -p` + `west flash`
