# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(Sound_detector)

if(NOT DEFINED SELECTED_PROJECT)
    message(FATAL_ERROR "Please specify a project to build with -DSELECTED_PROJECT=<PROJECT_NAME>")
endif()


if(SELECTED_PROJECT STREQUAL "SOUND_DETECTOR")
    target_sources(app PRIVATE Sound_detector/src/main.cpp Sound_detector/src/engine.cpp Sound_detector/src/nrf24.cpp Sound_detector/src/microphone.cpp)
    target_include_directories(app PRIVATE Sound_detector/include)
else()
    message(FATAL_ERROR "Unknown project: ${SELECTED_PROJECT}")
endif()