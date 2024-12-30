# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(Lidar)

if(NOT DEFINED SELECTED_PROJECT)
    message(FATAL_ERROR "Please specify a project to build with -DSELECTED_PROJECT=<PROJECT_NAME>")
endif()


if(SELECTED_PROJECT STREQUAL "LIDAR")
    target_sources(app PRIVATE Lidar/src/main.cpp Lidar/src/engine.cpp Lidar/src/nrf24.cpp Lidar/src/hcsr04.cpp)
    target_include_directories(app PRIVATE Lidar/include)
else()
    message(FATAL_ERROR "Unknown project: ${SELECTED_PROJECT}")
endif()