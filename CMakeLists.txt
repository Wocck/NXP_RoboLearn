# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(GPIO)

if(NOT DEFINED SELECTED_PROJECT)
    message(FATAL_ERROR "Please specify a project to build with -DSELECTED_PROJECT=<PROJECT_NAME>")
endif()


if(SELECTED_PROJECT STREQUAL "BUTTON")
    target_sources(app PRIVATE src/main_button.cpp)
elseif(SELECTED_PROJECT STREQUAL "GPIO")
    target_sources(app PRIVATE src/main_gpio.cpp)

else()
    message(FATAL_ERROR "Unknown project: ${SELECTED_PROJECT}")
endif()