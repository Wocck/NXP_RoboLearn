# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(Remote)

if(NOT DEFINED SELECTED_PROJECT)
    message(FATAL_ERROR "Please specify a project to build with -DSELECTED_PROJECT=<PROJECT_NAME>")
endif()


if(SELECTED_PROJECT STREQUAL "REMOTE")
    target_sources(app PRIVATE Remote_control/src/main.cpp Remote_control/src/nrf24.cpp Remote_control/src/engine.cpp)
    target_include_directories(app PRIVATE Remote_control/include)
else()
    message(FATAL_ERROR "Unknown project: ${SELECTED_PROJECT}")
endif()