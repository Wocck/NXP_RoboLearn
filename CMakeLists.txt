# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(LineFollower)

if(NOT DEFINED SELECTED_PROJECT)
    message(FATAL_ERROR "Please specify a project to build with -DSELECTED_PROJECT=<PROJECT_NAME>")
endif()


if(SELECTED_PROJECT STREQUAL "LINE_FOLLOWER")
    target_sources(app PRIVATE LineFollower/src/main.cpp LineFollower/src/line_follower.cpp LineFollower/src/engine.cpp)
    target_include_directories(app PRIVATE LineFollower/include)
else()
    message(FATAL_ERROR "Unknown project: ${SELECTED_PROJECT}")
endif()