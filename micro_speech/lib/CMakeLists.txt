# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.13.1)
find_package(Zephyr HINTS $ENV{ZEPHYR_BASE})
project(external_lib)


target_sources(app PRIVATE src/i2s.cc)
target_sources(app PRIVATE src/audio_provider.cc)
#target_sources(app PRIVATE src/audio_provider_mock.cc)
#target_sources(app PRIVATE src/audio_provider_mock_test.cc)
#target_sources(app PRIVATE src/audio_provider_test.cc)
target_sources(app PRIVATE src/command_responder.cc)
#target_sources(app PRIVATE src/command_responder_test.cc)
target_sources(app PRIVATE src/feature_provider.cc)
#target_sources(app PRIVATE src/feature_provider_mock_test.cc)
#target_sources(app PRIVATE src/feature_provider_test.cc)
target_sources(app PRIVATE src/main.cc)
target_sources(app PRIVATE src/main_functions.cc)
#target_sources(app PRIVATE src/micro_speech_test.cc)
target_sources(app PRIVATE src/no_1000ms_sample_data.cc)
target_sources(app PRIVATE src/no_30ms_sample_data.cc)
target_sources(app PRIVATE src/recognize_commands.cc)
#target_sources(app PRIVATE src/recognize_commands_test.cc)
target_sources(app PRIVATE src/yes_1000ms_sample_data.cc)
target_sources(app PRIVATE src/yes_30ms_sample_data.cc)


target_sources(app PRIVATE src/micro_features/micro_features_generator.cc)
#target_sources(app PRIVATE src/micro_features/micro_features_generator_test.cc)
target_sources(app PRIVATE src/micro_features/micro_model_settings.cc)
target_sources(app PRIVATE src/micro_features/model.cc)
target_sources(app PRIVATE src/micro_features/no_feature_data_slice.cc)
target_sources(app PRIVATE src/micro_features/no_micro_features_data.cc)
#target_sources(app PRIVATE src/micro_features/yes_feature_data_slice.cc)
target_sources(app PRIVATE src/micro_features/yes_micro_features_data.cc)


zephyr_include_directories(src src/micro_features) 

# The external static library that we are linking with does not know
# how to build for this platform so we export all the flags used in
# this zephyr build to the external build system.
#
# Other external build systems may be self-contained enough that they
# do not need any build information from zephyr. Or they may be
# incompatible with certain zephyr options and need them to be
# filtered out.


!!!ERROR!!!!                  # you need to fix the following path before you run the code
set(TF_SRC_DIR  YOUR_PATH_TO/nrf-tensorflow/tensorflow)
#set(TF_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../tensorflow)    #Somehow this does not work, and I have no idea why CMAKE does not like the relative path.
set(TF_MAKE_DIR ${TF_SRC_DIR}/tensorflow/lite/micro/tools/make)
# Create a wrapper CMake library that our app can link with
add_library(tf_lib STATIC IMPORTED GLOBAL)

#This library is only compiled for nRF5340 app core Cortex-m33.
set_target_properties(tf_lib PROPERTIES IMPORTED_LOCATION             ${CMAKE_SOURCE_DIR}/lib/nrf5340_cortex-m33_libtensorflow-microlite.a)
set_target_properties(tf_lib PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${TF_SRC_DIR};${TF_SRC_DIR}/tensorflow/lite/micro;${TF_MAKE_DIR}/downloads/flatbuffers/include") 


target_link_libraries(app PUBLIC tf_lib)
