# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(Interrupts)

if(NOT DEFINED SELECTED_PROJECT)
    message(FATAL_ERROR "Please specify a project to build with -DSELECTED_PROJECT=<PROJECT_NAME>")
endif()


if(SELECTED_PROJECT STREQUAL "BUTTON")
target_sources(app PRIVATE Projects/Button_interrupt/main.cpp)

elseif(SELECTED_PROJECT STREQUAL "NRF24")
    target_sources(app PRIVATE Projects/NRF24/src/main.cpp Projects/NRF24/src/nrf24.cpp)
    target_include_directories(app PRIVATE Projects/NRF24/include)

elseif(SELECTED_PROJECT STREQUAL "NRF24_EXAMPLE")
    target_sources(app PRIVATE Projects/NRF24_example/src/main.cpp Projects/NRF24_example/src/nrf24.cpp)
    target_include_directories(app PRIVATE Projects/NRF24_example/include)

else()
    message(FATAL_ERROR "Unknown project: ${SELECTED_PROJECT}")

endif()
