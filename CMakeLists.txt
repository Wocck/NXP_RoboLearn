# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(serial)

if(NOT DEFINED SELECTED_PROJECT)
    message(FATAL_ERROR "Please specify a project to build with -DSELECTED_PROJECT=<PROJECT_NAME>")
endif()

if(SELECTED_PROJECT STREQUAL "AHT40")
    target_sources(app PRIVATE Projects/AHT40/src/main.cpp Projects/AHT40/src/aht40.cpp)
    target_include_directories(app PRIVATE Projects/AHT40/include)
elseif(SELECTED_PROJECT STREQUAL "ST1140")
    target_sources(app PRIVATE Projects/ST1140/main.cpp)
elseif(SELECTED_PROJECT STREQUAL "HCSR04")
    target_sources(app PRIVATE Projects/HCSR04/main.cpp)
elseif(SELECTED_PROJECT STREQUAL "NRF24L01")
    target_sources(app PRIVATE Projects/NRF24L01/src/main.cpp Projects/NRF24L01/src/nrf24.cpp)
    target_include_directories(app PRIVATE Projects/NRF24L01/include)
elseif(SELECTED_PROJECT STREQUAL "UART")
    target_sources(app PRIVATE Projects/UART/src/main.cpp Projects/UART/src/nrf24.cpp)
    target_include_directories(app PRIVATE Projects/UART/include)
else()
    message(FATAL_ERROR "Unknown project: ${SELECTED_PROJECT}")
endif()
